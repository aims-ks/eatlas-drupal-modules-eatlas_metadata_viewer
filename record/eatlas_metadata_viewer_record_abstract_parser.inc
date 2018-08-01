<?php

class EAtlas_metadata_viewer_record_abstract_parser {
	private $schema_id;
	private $record_url;
	private $uuid;
	private $published;
	private $categories;

	// XML fields
	// Every fields that can be set in the record may appear multiple times,
	// even title and description. See Dublin core...
	private $preview_images;
	private $titles;
	private $desctiptions;
	private $purposes;
	private $supplemental_informations;
	private $links;
	private $keywords;

	// Parents
	// NOTE: Children are collected using the search index. See the module file
	private $parent_records;

	// Resource constraints
	private $use_limitations;
	private $access_constraints;
	private $use_constraints;
	private $other_constraints;

	// Responsible parties
	private $authors;
	private $owners;
	private $custodians;
	private $distributors;
	private $points_of_contact;
	private $metadata_contacts;
	private $principal_investigators;
	private $co_investigators;

	// Temporal extents
	// NOTE: Each Spacial extend may be assiciated with a Temporal extent.
	//   We don't care about this. There is only one map in the viewer.
	//   We usualy set only one temporal extents for the whole record.
	//   All temporal extents are grouped together.
	private $temporal_extents;

	// Geographical elements
	// NOTE: WMS and KML layers are in $links
	//   See getWmsLinks and getKmlLinks
	private $bboxes; // Bounding boxes
	private $polygons;
	private $points;

	public function __construct($schema_id, $xml_str, $uuid, $categories, $published) {
		$base_url = $this->getBaseURL();

		$this->schema_id = $schema_id;
		$this->uuid = $uuid;
		$this->categories = $categories;
		$this->setPublished($published);

		$this->setRecordUrl(url(
			$base_url . 'metadata.show',
			array(
				'query' => array(
					'uuid' => $uuid,
					'currTab' => 'complete'
				)
			)
		));

		$this->parseXml($xml_str);
	}

	// Implemented in extended classes
	protected function parseXml($xml_str) {}

	protected function getBaseURL() {
		// TODO Get rid of 'http://eatlas.org.au'
		return 'http://eatlas.org.au' . '/geonetwork/srv/eng/';
	}

	public function getSchemaId() {
		return $this->schema_id;
	}

	public function getUUID() {
		return $this->uuid;
	}

	public function getCategories() {
		return $this->categories;
	}

	public function getRecordUrl() {
		return $this->record_url;
	}
	public function setRecordUrl($url) {
		$this->record_url = $url;
	}

	public function isPublished() {
		return $this->published;
	}
	public function setPublished($published) {
		$this->published = (bool)$published;
	}

	public function getPreviewImages() {
		return $this->preview_images;
	}
	public function addPreviewImage($url, $description, $width = NULL, $height = NULL) {
		if ($this->preview_images === NULL) {
			$this->preview_images = array();
		}
		// TODO Download image and inspect size?
		if ($width === NULL && $height === NULL) {
			$width = 180;
		}
		if (empty($url)) {
			return 'Invalid preview image.';
		}

		$this->preview_images[] = array(
			'url' => $url,
			'description' => $description,
			'width' => $width,
			'height' => $height
		);
	}

	public function getTitles() {
		return $this->titles;
	}
	public function addTitle($title) {
		if ($this->titles === NULL) {
			$this->titles = array();
		}
		$this->titles[] = $title;
	}

	public function getDesctiptions() {
		return $this->desctiptions;
	}
	public function addDesctiption($desctiption) {
		if ($this->desctiptions === NULL) {
			$this->desctiptions = array();
		}
		$this->desctiptions[] = $desctiption;
	}

	public function getPurposes() {
		return $this->purposes;
	}
	public function addPurpose($purpose) {
		if ($this->purposes === NULL) {
			$this->purposes = array();
		}
		$this->purposes[] = $purpose;
	}

	public function getSupplementalInformations() {
		return $this->supplemental_informations;
	}
	public function addSupplementalInformation($supplemental_information) {
		if ($this->supplemental_informations === NULL) {
			$this->supplemental_informations = array();
		}
		$this->supplemental_informations[] = $supplemental_information;
	}

	public function getLinks() {
		return $this->links;
	}
	public function addLink($url, $name = NULL, $description = NULL, $type = NULL) {
		if ($this->links === NULL) {
			$this->links = array();
		}
		if (empty($url)) {
			return 'Invalid link.';
		}

		$this->links[] = array(
			'url' => $url,
			'name' => $name,
			'description' => $description,
			'type' => $type
		);
	}

	// Get links matching certains types.
	// Should be overriden
	public function getWmsLinks() {
		return array();
	}
	public function getKmlLinks() {
		return array();
	}
	public function getRelatedLinks() {
		return array();
	}
	public function getPublicationLinks() {
		return array();
	}
	// $reverse: When set to TRUE, return all links NOT matching the given types.
	public function getLinksByTypes($types, $reverse = FALSE) {
		if (empty($types) || !is_array($types)) {
			return array();
		}
		$links = array();
		if (!empty($this->links)) {
			foreach($this->links as $link) {
				$in_array = in_array($link['type'], $types);
				if ($reverse) {
					if (!$in_array) {
						$links[] = $link;
					}
				} else {
					if ($in_array) {
						$links[] = $link;
					}
				}
			}
		}
		return $links;
	}

	public function getKeywords() {
		return $this->keywords;
	}
	public function addKeyword($keyword) {
		if ($this->keywords === NULL) {
			$this->keywords = array();
		}
		$this->keywords[] = $keyword;
	}


	// Parents

	public function getParentRecords() {
		return $this->parent_records;
	}
	public function addParentRecord($parent_record) {
		if ($this->parent_records === NULL) {
			$this->parent_records = array();
		}
		$this->parent_records[] = $parent_record;
	}


	// Resource constraints

	public function getUseLimitations() {
		return $this->use_limitations;
	}
	public function addUseLimitation($use_limitation) {
		if ($this->use_limitations === NULL) {
			$this->use_limitations = array();
		}
		$this->use_limitations[] = $use_limitation;
	}

	public function getAccessConstraints() {
		return $this->access_constraints;
	}
	public function addAccessConstraint($access_constraint) {
		if ($this->access_constraints === NULL) {
			$this->access_constraints = array();
		}
		$this->access_constraints[] = $access_constraint;
	}

	public function getUseConstraints() {
		return $this->use_constraints;
	}
	public function addUseConstraint($use_constraint) {
		if ($this->use_constraints === NULL) {
			$this->use_constraints = array();
		}
		$this->use_constraints[] = $use_constraint;
	}

	public function getOtherConstraints() {
		return $this->other_constraints;
	}
	public function addOtherConstraint($other_constraint) {
		if ($this->other_constraints === NULL) {
			$this->other_constraints = array();
		}
		$this->other_constraints[] = $other_constraint;
	}


	// Responsible parties

	public function getAuthors() {
		return $this->authors;
	}
	public function addAuthor($names, $positions = NULL, $organisations = NULL, $emails = NULL) {
		if ($this->authors === NULL) {
			$this->authors = array();
		}
		$this->authors[] = array(
			'names' => $names,
			'positions' => $positions,
			'organisations' => $organisations,
			'emails' => $emails
		);
	}

	public function getOwners() {
		return $this->owners;
	}
	public function addOwner($names, $positions = NULL, $organisations = NULL, $emails = NULL) {
		if ($this->owners === NULL) {
			$this->owners = array();
		}
		$this->owners[] = array(
			'names' => $names,
			'positions' => $positions,
			'organisations' => $organisations,
			'emails' => $emails
		);
	}

	public function getCustodians() {
		return $this->custodians;
	}
	public function addCustodian($names, $positions = NULL, $organisations = NULL, $emails = NULL) {
		if ($this->custodians === NULL) {
			$this->custodians = array();
		}
		$this->custodians[] = array(
			'names' => $names,
			'positions' => $positions,
			'organisations' => $organisations,
			'emails' => $emails
		);
	}

	public function getDistributors() {
		return $this->distributors;
	}
	public function addDistributor($names, $positions = NULL, $organisations = NULL, $emails = NULL) {
		if ($this->distributors === NULL) {
			$this->distributors = array();
		}
		$this->distributors[] = array(
			'names' => $names,
			'positions' => $positions,
			'organisations' => $organisations,
			'emails' => $emails
		);
	}

	public function getPointsOfContact() {
		return $this->points_of_contact;
	}
	public function addPointOfContact($names, $positions = NULL, $organisations = NULL, $emails = NULL) {
		if ($this->points_of_contact === NULL) {
			$this->points_of_contact = array();
		}
		$this->points_of_contact[] = array(
			'names' => $names,
			'positions' => $positions,
			'organisations' => $organisations,
			'emails' => $emails
		);
	}

	public function getMetadataContacts() {
		return $this->metadata_contacts;
	}
	public function addMetadataContact($names, $positions = NULL, $organisations = NULL, $emails = NULL) {
		if ($this->metadata_contacts === NULL) {
			$this->metadata_contacts = array();
		}
		$this->metadata_contacts[] = array(
			'names' => $names,
			'positions' => $positions,
			'organisations' => $organisations,
			'emails' => $emails
		);
	}

	public function getPrincipalInvestigators() {
		return $this->principal_investigators;
	}
	public function addPrincipalInvestigator($names, $positions = NULL, $organisations = NULL, $emails = NULL) {
		if ($this->principal_investigators === NULL) {
			$this->principal_investigators = array();
		}
		$this->principal_investigators[] = array(
			'names' => $names,
			'positions' => $positions,
			'organisations' => $organisations,
			'emails' => $emails
		);
	}

	public function getCoInvestigators() {
		return $this->co_investigators;
	}
	public function addCoInvestigator($names, $positions = NULL, $organisations = NULL, $emails = NULL) {
		if ($this->co_investigators === NULL) {
			$this->co_investigators = array();
		}
		$this->co_investigators[] = array(
			'names' => $names,
			'positions' => $positions,
			'organisations' => $organisations,
			'emails' => $emails
		);
	}


	// Temporal extents

	public function getTemporalExtents($sorted = TRUE) {
		if (empty($this->temporal_extents)) {
			return array();
		}

		// Remove duplicates
#		$temporal_extents = array_unique($this->temporal_extents);
		$temporal_extents = $this->temporal_extents;
		// Sort
		if ($sorted) {
			// Sort temporal extents by chronological order.
			// http://php.net/manual/en/function.usort.php
			usort($temporal_extents, function($a, $b) {
				if ($a['begin'] == $b['begin']) {
					return empty($a['end']) ? 0 : $a['end'] - empty($b['end']) ? 0 : $b['end'];
				}
				return $a['begin'] - $b['begin'];
			});
		}
		return $temporal_extents;
	}
	public function addTemporalExtent($begin_timestamp, $end_timestamp = NULL) {
		if ($this->temporal_extents === NULL) {
			$this->temporal_extents = array();
		}

		// Begin timestamp is mandatory
		if (!is_numeric($begin_timestamp)) {
			return 'Invalid begin timestamp.';
		}
		// End timestamp is optional
		if (!empty($end_timestamp) && !is_numeric($end_timestamp)) {
			return 'Invalid end timestamp.';
		}

		if ($begin_timestamp && $end_timestamp) {
			if ($begin_timestamp === $end_timestamp) {
				// If they are both equals, no need for a end.
				$end_timestamp = NULL;
			} elseif ($begin_timestamp > $end_timestamp) {
				// If it ends before it started, swap timestamps.
				$tmp = $begin_timestamp;
				$begin_timestamp = $end_timestamp;
				$end_timestamp = $tmp;
			}
		}

		$this->temporal_extents[] = array(
			'begin' => $begin_timestamp,
			'end' => $end_timestamp,
		);
	}


	// Points + Polygons = bbeba305-16f1-4008-9e32-970e7d1c19b7

	public function getBBoxes() {
		return $this->bboxes;
	}
	public function addBBox($bbox, $description = NULL) {
		if (!EAtlas_metadata_viewer_record_abstract_parser::isValidBBox($bbox)) {
			return 'Invalid bounding box.';
		}
		if ($this->bboxes === NULL) {
			$this->bboxes = array();
		}
		$this->bboxes[] = array(
			'bbox' => $bbox,
			'description' => $description
		);
	}

	public function getPolygons() {
		return $this->polygons;
	}
	public function addPolygon($polygon, $description = NULL) {
		if (!EAtlas_metadata_viewer_record_abstract_parser::isValidPolygon($polygon)) {
			return 'Invalid polygon.';
		}
		if ($this->polygons === NULL) {
			$this->polygons = array();
		}
		$this->polygons[] = array(
			'polygon' => $polygon,
			'description' => $description
		);
	}

	public function getPoints() {
		return $this->points;
	}
	public function addPoint($point, $description = NULL) {
		if (!EAtlas_metadata_viewer_record_abstract_parser::isValidPoint($point)) {
			return 'Invalid point.';
		}
		if ($this->points === NULL) {
			$this->points = array();
		}
		$this->points[] = array(
			'point' => $point,
			'description' => $description
		);
	}

	public static function isValidBBox(&$bbox) {
		if (!is_array($bbox)) {
			return FALSE;
		}
		if (!isset($bbox['north'])     || !is_numeric($bbox['north']) ||
				!isset($bbox['south']) || !is_numeric($bbox['south']) ||
				!isset($bbox['east'])  || !is_numeric($bbox['east']) ||
				!isset($bbox['west'])  || !is_numeric($bbox['west'])) {
			return FALSE;
		}

		return TRUE;
	}

	public static function isValidPolygon(&$polygon) {
		if (!is_array($polygon)) {
			return FALSE;
		}
		foreach($polygon as $point) {
			if (!EAtlas_metadata_viewer_record_abstract_parser::isValidPoint($point)) {
				return FALSE;
			}
		}

		// Ensure closure
		$first_point = $polygon[0];
		$last_point = $polygon[count($polygon) - 1];
		if ($first_point['lon'] !== $last_point['lon'] ||
				$first_point['lat'] !== $last_point['lat']) {
			$polygon[] = $first_point;
		}

		return TRUE;
	}

	public static function isValidPoint(&$point) {
		if (!is_array($point)) {
			return FALSE;
		}
		if (!isset($point['lon']) || !is_numeric($point['lon']) ||
				!isset($point['lat']) || !is_numeric($point['lat'])) {
			return FALSE;
		}

		return TRUE;
	}
}

?>
