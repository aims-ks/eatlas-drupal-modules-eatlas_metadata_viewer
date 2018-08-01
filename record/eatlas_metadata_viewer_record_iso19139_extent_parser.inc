<?php
/**
 * Temporal extent is a big nightmare.
 * It deserve it's own parser.
 */

class EAtlas_metadata_viewer_record_iso19139_extent_parser {
	private $record;

	public function __construct(EAtlas_metadata_viewer_record_abstract_parser $record) {
		$this->record = $record;
	}

	public function parseExtent($xml_extent) {
		foreach ($xml_extent->children('gmd', TRUE)->EX_Extent as $xml_EX_Extent) {

			$description = NULL;
			foreach ($xml_EX_Extent->children('gmd', TRUE)->description as $xml_description) {
				foreach ($xml_description->children('gco', TRUE)->CharacterString as $xml_CharacterString) {
					if (!empty($xml_CharacterString)) {
						$description = (string)$xml_CharacterString;
					}
				}
			}

			foreach ($xml_EX_Extent->children('gmd', TRUE)->geographicElement as $xml_geographicElement) {
				$this->parseGeographicElement($xml_geographicElement, $description);
			}

			foreach ($xml_EX_Extent->children('gmd', TRUE)->temporalElement as $xml_temporalElement) {
				$this->parseTemporalElement($xml_temporalElement, $description);
			}
		}
	}

	private function parseGeographicElement($xml_geographicElement, $description) {
		foreach ($xml_geographicElement->children('gmd', TRUE)->EX_BoundingPolygon as $xml_EX_BoundingPolygon) {
			foreach ($xml_EX_BoundingPolygon->children('gmd', TRUE)->polygon as $xml_polygon) {
				foreach ($xml_polygon->children('gml', TRUE)->Polygon as $xml_gmlPolygon) {
					foreach ($xml_gmlPolygon->children('gml', TRUE)->exterior as $xml_exterior) {
						foreach ($xml_exterior->children('gml', TRUE)->LinearRing as $xml_LinearRing) {
							foreach ($xml_LinearRing->children('gml', TRUE)->coordinates as $xml_coordinates) {
								if (!empty($xml_coordinates)) {
									$polygon = $this->parseCoordinatesPolygon($xml_coordinates);
									if ($polygon) {
										$this->record->addPolygon($polygon, $description);
									}
								}
							}
							foreach ($xml_LinearRing->children('gml', TRUE)->posList as $xml_posList) {
								if (!empty($xml_posList)) {
									$polygon = $this->parsePosListPolygon($xml_posList);
									if ($polygon) {
										$this->record->addPolygon($polygon, $description);
									}
								}
							}
						}
					}
				}
			}
		}

		foreach ($xml_geographicElement->children('gmd', TRUE)->EX_GeographicBoundingBox as $xml_EX_GeographicBoundingBox) {
			$bbox = array();
			foreach ($xml_EX_GeographicBoundingBox->children('gmd', TRUE)->westBoundLongitude as $xml_westBoundLongitude) {
				foreach ($xml_westBoundLongitude->children('gco', TRUE)->Decimal as $xml_Decimal) {
					if (!empty($xml_Decimal)) {
						$bbox['west'] = (string)$xml_Decimal;
					}
				}
			}

			foreach ($xml_EX_GeographicBoundingBox->children('gmd', TRUE)->eastBoundLongitude as $xml_eastBoundLongitude) {
				foreach ($xml_eastBoundLongitude->children('gco', TRUE)->Decimal as $xml_Decimal) {
					if (!empty($xml_Decimal)) {
						$bbox['east'] = (string)$xml_Decimal;
					}
				}
			}

			foreach ($xml_EX_GeographicBoundingBox->children('gmd', TRUE)->southBoundLatitude as $xml_southBoundLatitude) {
				foreach ($xml_southBoundLatitude->children('gco', TRUE)->Decimal as $xml_Decimal) {
					if (!empty($xml_Decimal)) {
						$bbox['south'] = (string)$xml_Decimal;
					}
				}
			}

			foreach ($xml_EX_GeographicBoundingBox->children('gmd', TRUE)->northBoundLatitude as $xml_northBoundLatitude) {
				foreach ($xml_northBoundLatitude->children('gco', TRUE)->Decimal as $xml_Decimal) {
					if (!empty($xml_Decimal)) {
						$bbox['north'] = (string)$xml_Decimal;
					}
				}
			}

			if (isset($bbox['west']) && isset($bbox['east']) &&
					isset($bbox['south']) && isset($bbox['north'])) {
				if ($bbox['west'] === $bbox['east'] && $bbox['south'] === $bbox['north']) {
					$this->record->addPoint(array(
						'lon' => $bbox['east'],
						'lat' => $bbox['north']
					), $description);
				} else {
					$this->record->addBBox($bbox, $description);
				}
			}
		}
	}

	private function parseTemporalElement($xml_temporalElement, $description) {
		foreach ($xml_temporalElement->children('gmd', TRUE)->EX_SpatialTemporalExtent as $xml_gmd_EX_SpatialTemporalExtent) {
			$this->parseExSpatialTemporalExtent($xml_gmd_EX_SpatialTemporalExtent, $description);
		}

		foreach ($xml_temporalElement->children('gmd', TRUE)->EX_TemporalExtent as $xml_gmd_EX_TemporalExtent) {
			$this->parseExTemporalExtent($xml_gmd_EX_TemporalExtent);
		}

		foreach ($xml_temporalElement->children('mcp', TRUE)->EX_TemporalExtent as $xml_mcp_EX_TemporalExtent) {
			$this->parseExTemporalExtent($xml_mcp_EX_TemporalExtent);
		}
	}

	private function parseExSpatialTemporalExtent($xml_EX_TemporalExtent, $description) {
		foreach ($xml_EX_TemporalExtent->children('gmd', TRUE)->extent as $xml_extent) {
			$this->parseTemporalExtent($xml_extent);
		}

		foreach ($xml_EX_TemporalExtent->children('gmd', TRUE)->spatialExtent as $xml_spatialExtent) {
			$this->parseGeographicElement($xml_spatialExtent, $description);
		}
	}

	private function parseExTemporalExtent($xml_EX_TemporalExtent) {
		foreach ($xml_EX_TemporalExtent->children('gmd', TRUE)->extent as $xml_extent) {
			$this->parseTemporalExtent($xml_extent);
		}
	}

	private function parseTemporalExtent($xml_extent) {
		foreach ($xml_extent->children('gml', TRUE)->TimeEdge as $xml_TimeEdge) {
			$this->parseTimeEdge($xml_TimeEdge);
		}

		foreach ($xml_extent->children('gml', TRUE)->TimeInstant as $xml_TimeInstant) {
			$date = $this->parseTimeInstant($xml_TimeInstant);
			if ($date) {
				$this->record->addTemporalExtent($date);
			}
		}

		foreach ($xml_extent->children('gml', TRUE)->TimeNode as $xml_TimeNode) {
			$this->parseTimeNode($xml_TimeNode);
		}

		foreach ($xml_extent->children('gml', TRUE)->TimePeriod as $xml_TimePeriod) {
			$this->parseTimePeriod($xml_TimePeriod);
		}
	}

	private function parseTimeEdge($xml_TimeEdge) {
		foreach ($xml_TimeEdge->children('gml', TRUE)->relatedTime as $xml_relatedTime) {
			$this->parseTemporalExtent($xml_relatedTime);
		}
		foreach ($xml_TimeEdge->children('gml', TRUE)->start as $xml_start) {
			$this->parseTemporalExtent($xml_start);
		}
		foreach ($xml_TimeEdge->children('gml', TRUE)->end as $xml_end) {
			$this->parseTemporalExtent($xml_end);
		}
	}

	private function parseTimeNode($xml_TimeNode) {
		foreach ($xml_TimeNode->children('gml', TRUE)->relatedTime as $xml_relatedTime) {
			$this->parseTemporalExtent($xml_relatedTime);
		}
		foreach ($xml_TimeNode->children('gml', TRUE)->position as $xml_position) {
			foreach ($xml_position->children('gml', TRUE)->TimeInstant as $xml_TimeInstant) {
				$timestamp = $this->parseTimeInstant($xml_TimeInstant);
				$this->record->addTemporalExtent($timestamp);
			}
		}
		foreach ($xml_TimeNode->children('gml', TRUE)->previousEdge as $xml_previousEdge) {
			$this->parseTemporalExtent($xml_previousEdge);
		}
		foreach ($xml_TimeNode->children('gml', TRUE)->nextEdge as $xml_nextEdge) {
			$this->parseTemporalExtent($xml_nextEdge);
		}
	}

	private function parseTimePeriod($xml_TimePeriod) {
		$begin = NULL;
		$end = NULL;

		foreach ($xml_TimePeriod->children('gml', TRUE)->beginPosition as $xml_beginPosition) {
			$begin = strtotime((string)$xml_beginPosition);
		}
		if ($begin === NULL) {
			foreach ($xml_TimePeriod->children('gml', TRUE)->begin as $xml_begin) {
				foreach ($xml_begin->children('gml', TRUE)->TimeInstant as $xml_TimeInstant) {
					$begin = $this->parseTimeInstant($xml_TimeInstant);
				}
			}
		}

		foreach ($xml_TimePeriod->children('gml', TRUE)->endPosition as $xml_endPosition) {
			$end = strtotime((string)$xml_endPosition);
		}
		if ($end === NULL) {
			foreach ($xml_TimePeriod->children('gml', TRUE)->end as $xml_end) {
				foreach ($xml_end->children('gml', TRUE)->TimeInstant as $xml_TimeInstant) {
					$end = $this->parseTimeInstant($xml_TimeInstant);
				}
			}
		}

		if ($begin || $end) {
			$this->record->addTemporalExtent($begin, $end);
		}
	}

	// Return the first date within the TimeInstant object
	private function parseTimeInstant($xml_TimeInstant) {
		foreach ($xml_TimeInstant->children('gml', TRUE)->timePosition as $xml_timePosition) {
			if (!empty($xml_timePosition)) {
				return strtotime((string)$xml_timePosition);
			}
		}
		return NULL;
	}


	private function parseCoordinatesPolygon($xml_polygon) {
		// 145.0010529126619,-10.68188894877146,0 142.5315075081651,-10.68756998592151,0 142.7881039791155,-11.08215241694377,0 142.8563635161634,-11.84494938862347,0 ...
		$polygon = array();
		$point_list = explode(' ', (string)$xml_polygon);
		$point_list_count = count($point_list);
		for($i=0; $i<$point_list_count; $i++) {
			$point = explode(',', $point_list[$i]);
			$polygon[] = array(
				'lon' => $point[0],
				'lat' => $point[1]
			);
		}
		return $polygon;


		return NULL;
	}

	private function parsePosListPolygon($xml_polygon) {
		// 151.083984375 -24.521484375 153.80859375 -24.521484375 153.45703125 -20.830078125 147.12890625 -17.490234375 145.810546875 -13.798828125 144.4921875 -12.832031250000002 ...
		$polygon = array();
		$pos_list = explode(' ', (string)$xml_polygon);
		$pos_list_count = count($pos_list);
		if ($pos_list_count > 0 && $pos_list_count % 2 === 0) {
			for($i=0; $i<$pos_list_count; $i+=2) {
				$polygon[] = array(
					'lon' => $pos_list[$i],
					'lat' => $pos_list[$i+1]
				);
			}
			return $polygon;
		}
		return NULL;
	}
}

?>
