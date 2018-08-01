<?php
module_load_include('inc', 'eatlas_metadata_viewer', 'record/eatlas_metadata_viewer_record_abstract_parser');

class EAtlas_metadata_viewer_record_dublin_core_parser extends EAtlas_metadata_viewer_record_abstract_parser {

	protected function parseXml($xml_str) {
		// Parse the XML document
		$xml = new SimpleXMLElement($xml_str);

		foreach ($xml->children('dc', TRUE)->title as $xml_title) {
			if (!empty($xml_title)) {
				$this->addTitle((string)$xml_title);
			}
		}

		foreach ($xml->children('dc', TRUE)->description as $xml_description) {
			if (!empty($xml_description)) {
				$this->addDesctiption((string)$xml_description);
			}
		}

		foreach ($xml->children('dc', TRUE)->creator as $xml_creator) {
			if (!empty($xml_creator)) {
				$this->addAuthor(array((string)$xml_creator));
			}
		}

		foreach ($xml->children('dc', TRUE)->publisher as $xml_publisher) {
			if (!empty($xml_publisher)) {
				$this->addCustodian(array((string)$xml_publisher));
			}
		}

		foreach ($xml->children('dc', TRUE)->rights as $xml_rights) {
			if (!empty($xml_rights)) {
				$this->addUseConstraint((string)$xml_rights);
			}
		}

		foreach ($xml->children('dc', TRUE)->subject as $xml_subject) {
			if (!empty($xml_subject)) {
				$this->addKeyword((string)$xml_subject);
			}
		}

		foreach ($xml->children('dc', TRUE)->relation as $xml_relation) {
			if (!empty($xml_relation)) {
				$this->addLink((string)$xml_relation);
			}
		}

		foreach ($xml->children('dc', TRUE)->coverage as $xml_coverage) {
			if (!empty($xml_coverage)) {
				$coverage = $this->parseCoverage((string)$xml_coverage);
				if ($coverage) {
					$this->addPolygon($coverage);
				}
			}
		}
	}

	// The format of coverage is free text.
	// Default value is: "North 90, South -90, East 180, West -180. Global"
	// I'm not sure if it worth trying to parse it...
	// This should return a polygon.
	private function parseCoverage($coverage) {
		return NULL;
	}
}

?>
