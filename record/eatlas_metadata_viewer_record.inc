<?php

// "net_buffer_length" has a physical (hardcoded) limit of "1 048 576" bytes (1MB) in MySQL.
//     https://dev.mysql.com/doc/refman/5.0/en/server-system-variables.html#sysvar_net_buffer_length
//
// NOTE: The query return a max of 1MB per fields. I'm not 100% sure this
//     limit is set by "net_buffer_length" or another MySQL (or PHP) variable.
//     When executing the query in the MySQL terminal, the record is NOT
//     truncated. That kind of tell me that it's something to do with PHP.
define('EATLAS_METADATA_VIEWER_MAX_BUFFER_LENGTH', 1024*1024);

function _eatlas_metadata_viewer_record_view($uuid) {
	$record = eatlas_metadata_viewer_load_record($uuid);

	if (empty($record)) {
		// The record do not exists
		// https://api.drupal.org/api/drupal/includes!common.inc/function/drupal_not_found/7
		return drupal_not_found();
	}

	if (module_exists('eatlas_branding') && function_exists('eatlas_branding_geonetwork_categories_is_supported')) {
		if (!eatlas_branding_geonetwork_categories_is_supported($record->getCategories())) {
			// The record do not support the current branding
			return drupal_not_found();
		}

		// TODO Set the canonical link
		$canonical_link = eatlas_branding_geonetwork_record_get_canonical_url($record->getCategories());
		if ($canonical_link) {
			drupal_add_html_head_link(array('rel' => 'canonical', 'href' => $canonical_link));
		}
	}

	if (!$record->isPublished() && !user_access('view unpublished eatlas metadata records')) {
		// The record is unpublished AND the user is not allowed to see unpublished records
		// https://api.drupal.org/api/drupal/includes%21common.inc/function/drupal_access_denied/7
		return drupal_access_denied();
	}


	// Load the map library (Leaflet)
	$map_libraries = eatlas_metadata_viewer_get_map_library();
	$map_library_found = FALSE;
	foreach($map_libraries as $map_library) {
		if ($map_library['version'] !== NULL) {
			drupal_add_js($map_library['path'], array('group' => JS_LIBRARY));
			if (isset($map_library['plugins'])) {
				foreach($map_library['plugins'] as $plugin_file) {
					drupal_add_js($plugin_file, array('group' => JS_LIBRARY));
				}
			}
			if (isset($map_library['css'])) {
				foreach($map_library['css'] as $css_file) {
					drupal_add_css($css_file);
				}
			}
			$map_library_found = TRUE;
			break;
		}
	}

	// Send the map settings to JavaScript
	// (map center, layers, polygons, points, bounding boxes, etc).
	// It will be used by js/eatlas_metadata_viewer_map.js
	if ($map_library_found) {
		$settings = eatlas_metadata_viewer_load_settings();
		drupal_add_js(array('eatlas_metadata_viewer' => array(
			'map_wms_server' => property_exists($settings, 'map_wms_server') ? $settings->map_wms_server : NULL,
			'map_wms_layer' => property_exists($settings, 'map_wms_layer') ? $settings->map_wms_layer : NULL,
			'map_centre_lat' => property_exists($settings, 'map_centre_lat') ? $settings->map_centre_lat : NULL,
			'map_centre_lon' => property_exists($settings, 'map_centre_lon') ? $settings->map_centre_lon : NULL,
			'map_zoom' => property_exists($settings, 'map_zoom') ? $settings->map_zoom : NULL,
			'bboxes' => $record->getBBoxes(),
			'polygons' => $record->getPolygons(),
			'points' => $record->getPoints(),
			'wms_links' => $record->getWmsLinks(),
			'kml_links' => $record->getKmlLinks()
		)), array('type' => 'setting'));
	}


	$titles = $record->getTitles();
	if (!empty($titles)) {
		$title = trim(implode(', ', $titles));
		drupal_set_title($title);
	}

	$variables = array();
	$variables['record'] = $record;
	$variables['child_records'] = eatlas_metadata_viewer_load_child_records($uuid);

	return theme('metadata_record', $variables);
}

function eatlas_metadata_viewer_load_record($uuid) {
	$settings = eatlas_metadata_viewer_load_settings();
	$record = NULL;

	$db_connection = _eatlas_metadata_viewer_get_geonetwork_connection($uuid, $settings);

	// Get the needed columns only.
	// *IMPORTANT*
	//     DO NOT get data, it breaks when the record is too big.
	//     Example: '5bb9a340-4ade-11dc-8f56-00008a07204e'
	$stmt = $db_connection->prepare('SELECT id, uuid, schemaId, CHAR_LENGTH(data) as data_len FROM metadata WHERE uuid = :uuid');
	$stmt->execute(array('uuid' => $uuid));

	foreach ($stmt as $row) {
		// mysql> DESCRIBE metadata;
		// +--------------+--------------+------+-----+---------+-------+
		// | Field        | Type         | Null | Key | Default | Extra |
		// +--------------+--------------+------+-----+---------+-------+
		// | id           | int(11)      | NO   | PRI | NULL    |       |
		// | uuid         | varchar(250) | NO   | UNI | NULL    |       |
		// | schemaId     | varchar(32)  | NO   |     | NULL    |       |
		// | isTemplate   | char(1)      | NO   |     | n       |       |
		// | isHarvested  | char(1)      | NO   |     | n       |       |
		// | createDate   | varchar(30)  | NO   |     | NULL    |       |
		// | changeDate   | varchar(30)  | NO   |     | NULL    |       |
		// | data         | longtext     | NO   |     | NULL    |       |
		// | source       | varchar(250) | NO   | MUL | NULL    |       |
		// | title        | varchar(255) | YES  |     | NULL    |       |
		// | root         | varchar(255) | YES  |     | NULL    |       |
		// | harvestUuid  | varchar(250) | YES  |     | NULL    |       |
		// | owner        | int(11)      | NO   | MUL | NULL    |       |
		// | doctype      | varchar(255) | YES  |     | NULL    |       |
		// | groupOwner   | int(11)      | YES  | MUL | NULL    |       |
		// | harvestUri   | varchar(512) | YES  |     | NULL    |       |
		// | rating       | int(11)      | NO   |     | 0       |       |
		// | popularity   | int(11)      | NO   |     | 0       |       |
		// | displayorder | int(11)      | YES  |     | NULL    |       |
		// +--------------+--------------+------+-----+---------+-------+
		// 19 rows in set (0.00 sec)

		// Ensure the record is NOT corrupted
		if (isset($row['schemaId']) && !empty($row['schemaId']) &&
				isset($row['data_len']) && !empty($row['data_len']) &&
				isset($row['uuid']) && !empty($row['uuid']) &&
				isset($row['id']) && !empty($row['id'])) {

			$categories = _eatlas_metadata_viewer_get_record_categories($row['id'], $db_connection);

			$privileges = _eatlas_metadata_viewer_get_record_privileges($row['id'], $db_connection);
			$is_published = _eatlas_metadata_viewer_is_record_published($privileges);

			// Get the data by segments of "EATLAS_METADATA_VIEWER_MAX_BUFFER_LENGTH" bytes.
			$data = '';
			for ($offset = 0; $offset < $row['data_len']; $offset += EATLAS_METADATA_VIEWER_MAX_BUFFER_LENGTH) {
				// Example of generated queries (for a pretty large record):
				//     SELECT SUBSTRING(data, 1, 1048576) AS data_part FROM metadata WHERE uuid = '5bb9a340-4ade-11dc-8f56-00008a07204e';
				//     SELECT SUBSTRING(data, 1048577, 1048576) AS data_part FROM metadata WHERE uuid = '5bb9a340-4ade-11dc-8f56-00008a07204e';
				$data_stmt = $db_connection->prepare('SELECT SUBSTRING(data, :offset, :max_buffer) AS data_part FROM metadata WHERE uuid = :uuid');
				$data_stmt->execute(array(
					'offset' => $offset + 1,
					'max_buffer' => EATLAS_METADATA_VIEWER_MAX_BUFFER_LENGTH,
					'uuid' => $uuid
				));

				// NOTE: There is only one row. The for loop is use to select it.
				foreach ($data_stmt as $data_row) {
					$data .= $data_row['data_part'];
				}
			}

			switch ($row['schemaId']) {
				case 'iso19139':
				case 'iso19139.anzlic':
				case 'iso19139.mcp':
				case 'iso19139.mcp-1.4':
					module_load_include('inc', 'eatlas_metadata_viewer', 'record/eatlas_metadata_viewer_record_iso19139_parser');
					$record = new EAtlas_metadata_viewer_record_iso19139_parser($row['schemaId'], $data, $row['uuid'], $categories, $is_published);
					break;
				case 'dublin-core':
					module_load_include('inc', 'eatlas_metadata_viewer', 'record/eatlas_metadata_viewer_record_dublin-core_parser');
					$record = new EAtlas_metadata_viewer_record_dublin_core_parser($row['schemaId'], $data, $row['uuid'], $categories, $is_published);
					break;
			}
		}
	}

	$db_connection = NULL;

	return $record;
}

// TODO IMPLEMENT THIS !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
// Get the child records from the index
function eatlas_metadata_viewer_load_child_records($uuid) {
	$child_records = array();
	return $child_records; // TODO Remove

	//for ($i=0; $i<count($child_records); $i++) {
		$child_records[] = array(
			'uuid' => '1a46774e-a3ac-4982-b08b-94ce1ad8d45c',
			'label' => 'ERP TE Project 1.1 Monitoring status and trends of coral reefs of the GBR (AIMS)'
		);
		$child_records[] = array(
			'uuid' => '79c0ce91-0b8d-4591-b960-d0af46b052ea',
			'label' => 'ERP TE Project 1.2 Marine wildlife management in the Great Barrier Reef World Heritage Area (JCU)'
		);
	//}

	return $child_records;
}

/**
 * Get the permissions for a given metadata ID (the integer ID, not the UUID).
 * Return the permissions as an array:
 *   privileges[<GROUP_ID>] = array(<OPERATION_ID>, <OPERATION_ID>, ...);
 * Example:
 *   privileges[0] = array(0, 1, 2, ...);
 *   privileges[1] = array(0, 1, 2, ...);
 *
 * NOTES:
 *   The privileges table looks like this in the GeoNetwork admin website:
 *     +--------------+---------+-------------+-----------------+-------------+-----------+-----------+-----------------------+
 *     | Groups       | Publish | Download    | Interactive Map | Feature     | Editing   | Notify    |                       |
 *     |              | "view"  | "download"  | "dynamic"       | "featured"  | "editing" | "notify"  |                       |
 *     +--------------+---------+-------------+-----------------+-------------+-----------+-----------+-----------------------+
 *     | All          |   [X]   |     [X]     |       [X]       |     [X]     |           |           | [Set All] [Clear All] |
 *     | Intranet     |   [ ]   |     [ ]     |       [ ]       |     [ ]     |           |           | [Set All] [Clear All] |
 *     | Guest        |   [ ]   |     [ ]     |       [ ]       |     [ ]     |           |           | [Set All] [Clear All] |
 *     +--------------+---------+-------------+-----------------+-------------+-----------+-----------+-----------------------+
 *     | NWAtlas      |   [ ]   |     [ ]     |       [ ]       |     [ ]     |    [ ]    |    [ ]    | [Set All] [Clear All] |
 *     | eAtlas       |   [X]   |     [ ]     |       [ ]       |     [ ]     |    [ ]    |    [X]    | [Set All] [Clear All] |
 *     | Sample group |   [ ]   |     [ ]     |       [ ]       |     [ ]     |    [ ]    |    [ ]    | [Set All] [Clear All] |
 *     | ...          |   [ ]   |     [ ]     |       [ ]       |     [ ]     |    [ ]    |    [ ]    | [Set All] [Clear All] |
 *     +--------------+---------+-------------+-----------------+-------------+-----------+-----------+-----------------------+
 *
 *   Database tables:
 *     Columns labels: Table "operations" (and "operationsdes" for translated labels)
 *       views, download, dynamic, etc.
 *       +----------+-------------+------+-----+---------+-------+
 *       | Field    | Type        | Null | Key | Default | Extra |
 *       +----------+-------------+------+-----+---------+-------+
 *       | id       | int(11)     | NO   | PRI | NULL    |       |
 *       | name     | varchar(32) | NO   |     | NULL    |       |
 *       | reserved | char(1)     | NO   |     | n       |       |
 *       +----------+-------------+------+-----+---------+-------+
 *
 *     Rows labels: Table "groups" (and "groupsdes" for translated labels)
 *       all, intranet, guest, etc.
 *       +-------------+--------------+------+-----+---------+-------+
 *       | Field       | Type         | Null | Key | Default | Extra |
 *       +-------------+--------------+------+-----+---------+-------+
 *       | id          | int(11)      | NO   | PRI | NULL    |       |
 *       | name        | varchar(32)  | NO   | UNI | NULL    |       |
 *       | description | varchar(255) | YES  |     | NULL    |       |
 *       | email       | varchar(128) | YES  |     | NULL    |       |
 *       | referrer    | int(11)      | YES  | MUL | NULL    |       |
 *       +-------------+--------------+------+-----+---------+-------+
 *
 *     Checkbox values: Table "operationallowed"
 *       +-------------+---------+------+-----+---------+-------+
 *       | Field       | Type    | Null | Key | Default | Extra |
 *       +-------------+---------+------+-----+---------+-------+
 *       | groupId     | int(11) | NO   | PRI | NULL    |       | FK: groups.id
 *       | metadataId  | int(11) | NO   | PRI | NULL    |       | FK: metadata.id
 *       | operationId | int(11) | NO   | PRI | NULL    |       | FK: operations.id
 *       +-------------+---------+------+-----+---------+-------+
 */
function _eatlas_metadata_viewer_get_record_privileges($metadata_id, $db_connection) {
	if (empty($metadata_id)) {
		return array();
	}

	// Friendly query for debugging:
	//   SELECT oa.*, g.name AS groupName, o.name AS operationName FROM operationallowed oa JOIN groups g ON oa.groupId = g.id JOIN operations o ON oa.operationId = o.id WHERE oa.metadataId = <METADATA_ID>;

	// Get the value of all checkboxes from the Privileges table (aka "operationallowed").
	$stmt = $db_connection->prepare('SELECT * FROM operationallowed WHERE metadataId = :metadata_id');
	$stmt->execute(array('metadata_id' => $metadata_id));

	$privileges = array();
	foreach ($stmt as $row) {
		$group_id = $row['groupId'];
		$operation_id = $row['operationId'];
		if (!isset($privileges[$group_id])) {
			$privileges[$group_id] = array();
		}
		$privileges[$group_id][] = $operation_id;
	}
	return $privileges;
}

/**
 * Returns TRUE if the record is published. FALSE otherwise.
 *
 * $privileges: Table returned by _eatlas_metadata_viewer_get_record_privileges()
 *
 * NOTE: After some testing, it turns out that the only checkbox
 *   that needs to be checked is the checkbox "Publish" (aka "view")
 *   for the group "All". None of the other checkboxes will make the
 *   record publicly available.
 */
function _eatlas_metadata_viewer_is_record_published($privileges) {
	$group_all_id = 1;
	$operation_view_id = 0;
	return (isset($privileges[$group_all_id]) && in_array($operation_view_id, $privileges[$group_all_id]));
}

/**
categories (and "categoriesdes" for translated labels)
+-------+--------------+------+-----+---------+-------+
| Field | Type         | Null | Key | Default | Extra |
+-------+--------------+------+-----+---------+-------+
| id    | int(11)      | NO   | PRI | NULL    |       |
| name  | varchar(255) | NO   | UNI | NULL    |       |
+-------+--------------+------+-----+---------+-------+

metadatacateg
+------------+---------+------+-----+---------+-------+
| Field      | Type    | Null | Key | Default | Extra |
+------------+---------+------+-----+---------+-------+
| metadataId | int(11) | NO   | PRI | NULL    |       | FK: metadata.id
| categoryId | int(11) | NO   | PRI | NULL    |       | FK: categories.id
+------------+---------+------+-----+---------+-------+
 */
function _eatlas_metadata_viewer_get_record_categories($metadata_id, $db_connection) {
	if (empty($metadata_id)) {
		return array();
	}

	// Get the value of all checkboxes from the Privileges table (aka "operationallowed").
	$stmt = $db_connection->prepare('SELECT c.name FROM metadatacateg m JOIN categories c ON c.id = m.categoryId WHERE m.metadataId = :metadata_id');
	$stmt->execute(array('metadata_id' => $metadata_id));

	$categories = array();
	foreach ($stmt as $row) {
		$categories[] = $row['name'];
	}
	return $categories;
}


function _eatlas_metadata_viewer_get_geonetwork_connection($uuid, $settings) {
	// Currently support MySQL only.
	// If required, more DB can be supported. The DB type will have to be
	//   specified in the admin interface, and pass to this function in
	//   the $settings object.
	return _eatlas_metadata_viewer_get_geonetwork_mysql_connection($uuid, $settings);
}

function _eatlas_metadata_viewer_get_geonetwork_mysql_connection($uuid, $settings) {
	$host = $settings->database_host;
	if (!$host) {
		$host = 'localhost';
	}

	// NOTE: The DSN is a URI. Variables needs to be escaped using "urlencode".
	$dsn = 'mysql:' .
		'dbname=' . urlencode($settings->database_name) . ';' .
		'host=' . urlencode($host) . ';' .
		'charset=utf8';

	// The username and password are managed by the PDO. No escape required.
	$db_connection = new PDO($dsn, $settings->database_user, $settings->database_pass);

	$db_connection->setAttribute(PDO::ATTR_EMULATE_PREPARES, false);
	$db_connection->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);

	return $db_connection;
}

?>
