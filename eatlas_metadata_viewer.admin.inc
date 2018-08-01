<?php

// NOTE: All those settings are saved in Drupal variables using "eatlas_metadata_viewer_" prefix.
//   They must be defined in "eatlas_metadata_viewer_create_empty_settings()"
function _eatlas_metadata_viewer_settings() {
	$settings = eatlas_metadata_viewer_load_settings();

	// https://api.drupal.org/api/drupal/developer%21topics%21forms_api_reference.html/7
	$form = array();

	$form['database_host'] = array(
		'#type' => 'textfield',
		'#title' => t('Database host'),
		'#description' => t('Host on which the GeoNetwork database reside. Use column separator to set a custom port "localhost:1234". If unsure, leave blank (default to "localhost").'),
		'#default_value' => property_exists($settings, 'database_host') ? $settings->database_host : ''
	);
	$form['database_name'] = array(
		'#type' => 'textfield',
		'#title' => t('Database name'),
		'#description' => t('Name of the database used by GeoNetwork.'),
		'#default_value' => property_exists($settings, 'database_name') ? $settings->database_name : '',
		'#required' => TRUE
	);
	$form['database_user'] = array(
		'#type' => 'textfield',
		'#title' => t('Database username'),
		'#description' => t('Username used for the connection to GeoNetwork database.'),
		'#default_value' => property_exists($settings, 'database_user') ? $settings->database_user : '',
		'#required' => TRUE
	);
	$form['database_pass'] = array(
		'#type' => 'password',
		'#title' => t('Database password'),
		'#description' => t('Password used for the connection to GeoNetwork database. Leave blank to leave that setting unchanged.'),
		// '#default_value' is disabled on password fields for security reasons.
		// If we really want to display it, there is the trick:
		//'#attributes' => array('value' => property_exists($settings, 'database_pass') ? $settings->database_pass : '')
	);
	$form['public_path'] = array(
		'#type' => 'textfield',
		'#title' => t('Public path'),
		'#description' => t('Path used to access the Metadata Records. The cache needs to be cleared after changing this setting.'),
		'#default_value' => property_exists($settings, 'public_path') ? $settings->public_path : 'data',
		'#required' => TRUE
	);

	// MAP SETTINGS

	$instructions = '';
	$map_libraries = eatlas_metadata_viewer_get_map_library();
	$has_map_library_installed = FALSE;
	foreach($map_libraries as $map_library) {
		$instructions .= '<div class="map-library">';
		$instructions .= '<label>' . $map_library['name'] . '</label>';
		if ($map_library['version'] !== NULL) {
			$instructions .= '<span class="version">';
			$instructions .= t('Installed version: !version', array('!version' => $map_library['version']));
			$instructions .= '</span>';
			$has_map_library_installed = TRUE;
		}
		$instructions .= '<ol class="instructions">';
		foreach($map_library['instructions'] as $instruction_step) {
			$instructions .= '<li>' . $instruction_step . '</li>';
		}
		$instructions .= '</ol>';
		$instructions .= '</div>';
	}

	$form['map_settings'] = array(
		'#type' => 'fieldset',
		'#title' => t('Map settings'),
		'#collapsible' => TRUE,
		'#collapsed' => !$has_map_library_installed,
		'#description' => 'Settings for the map displayed in the viewer, when there is spacial information attached to the record.'
	);

	$form['map_settings']['map_centre_lat'] = array(
		'#type' => 'textfield',
		'#title' => t('Map centre - latitude'),
		'#description' => t('Latitude of the centre of the map. Must be between -90 and 90.'),
		'#default_value' => property_exists($settings, 'map_centre_lat') ? $settings->map_centre_lat : '-18',
		'#required' => TRUE
	);
	$form['map_settings']['map_centre_lon'] = array(
		'#type' => 'textfield',
		'#title' => t('Map centre - longitude'),
		'#description' => t('Longitude of the centre of the map. Must be between -180 and 180.'),
		'#default_value' => property_exists($settings, 'map_centre_lon') ? $settings->map_centre_lon : '148',
		'#required' => TRUE
	);
	$form['map_settings']['map_zoom'] = array(
		'#type' => 'textfield',
		'#title' => t('Map zoom'),
		'#description' => t('Zoom level of the map. Must be between 0 and 20.'),
		'#default_value' => property_exists($settings, 'map_zoom') ? $settings->map_zoom : '5',
		'#required' => TRUE
	);

	$form['map_settings']['map_wms_server'] = array(
		'#type' => 'textfield',
		'#title' => t('WMS server'),
		'#description' => t('WMS server for the background layer of the map.<br/>Example: http://something.com/geoserver/wms'),
		'#default_value' => property_exists($settings, 'map_wms_server') ? $settings->map_wms_server : ''
	);
	$form['map_settings']['map_wms_layer'] = array(
		'#type' => 'textfield',
		'#title' => t('WMS layer'),
		'#description' => t('WMS layer name for the background of the map. Multiple layers may be provided (comma-separated).'),
		'#default_value' => property_exists($settings, 'map_wms_layer') ? $settings->map_wms_layer : ''
	);

	$form['installation_instructions'] = array(
		'#type' => 'fieldset',
		'#title' => t('Installation instructions'),
		'#collapsible' => TRUE,
		'#collapsed' => $has_map_library_installed,
		'#description' => (!$has_map_library_installed ? t('There are no map library installed. Install one of the following libraries if you want to have a map in the Metadata Viewer.') : '')
	);
	$form['installation_instructions']['steps'] = array('#markup' => $instructions);

	$form['actions'] = array('#type' => 'actions');
	$form['actions']['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Save')
	);

	return $form;
}

function _eatlas_metadata_viewer_settings_validate($form, &$form_state) {
	if (isset($form_state['values']['map_centre_lat'])) {
		$lat = $form_state['values']['map_centre_lat'];
		if (!is_numeric($lat)) {
			form_set_error('map_centre_lat', t('Latitude value must be numeric.'));
		} elseif ($lat < -90 || $lat > 90) {
			form_set_error('map_centre_lat', t('Latitude value excceed limits. Set a value between -90 and 90.'));
		}
	}
	if (isset($form_state['values']['map_centre_lon'])) {
		$lon = $form_state['values']['map_centre_lon'];
		if (!is_numeric($lon)) {
			form_set_error('map_centre_lon', t('Longitude value must be numeric.'));
		} elseif ($lon < -180 || $lon > 180) {
			form_set_error('map_centre_lon', t('Longitude value excceed limits. Set a value between -180 and 180.'));
		}
	}
	if (isset($form_state['values']['map_zoom'])) {
		$zoom = $form_state['values']['map_zoom'];
		if (!is_numeric($zoom)) {
			form_set_error('map_zoom', t('Zoom value must be numeric.'));
		} elseif ($zoom < 0 || $zoom > 20) {
			form_set_error('map_zoom', t('Zoom value excceed limits. Set a value between 0 and 20.'));
		}
	}
}

function _eatlas_metadata_viewer_settings_submit($form, &$form_state) {
	$settings = eatlas_metadata_viewer_create_empty_settings();

	foreach ($settings as $key => $value) {
		if (isset($form_state['values'][$key]) && !empty($form_state['values'][$key])) {
			$new_value = $form_state['values'][$key];
			// Trim the value, except for the password (it may start or end with spaces)
			if ($key !== 'database_pass') {
				$new_value = trim($new_value);
			}
			$settings->{$key} = $new_value;
		}
	}

	eatlas_metadata_viewer_save_settings($settings);
}

?>
