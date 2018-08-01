<?php
module_load_include('inc', 'eatlas_metadata_viewer', 'record/eatlas_metadata_viewer_record_abstract_parser');
module_load_include('inc', 'eatlas_metadata_viewer', 'record/eatlas_metadata_viewer_record_iso19139_extent_parser');

define('EATLAS_METADATA_DATA_LINK_PROTOCOL', 'WWW:LINK-1.0-http--link');
define('EATLAS_METADATA_FILE_LINK_PROTOCOL', 'WWW:DOWNLOAD-1.0-http--download');
define('EATLAS_METADATA_WMS_GM_LINK_PROTOCOL', 'OGC:WMS-1.1.1-http-get-map');
define('EATLAS_METADATA_KML_LINK_PROTOCOL', 'GLG:KML-2.0-http-get-map');
define('EATLAS_METADATA_RELATED_PROTOCOL', 'WWW:LINK-1.0-http--related');
define('EATLAS_METADATA_POINT_OF_TRUTH', 'WWW:LINK-1.0-http--metadata-URL');


class EAtlas_metadata_viewer_record_iso19139_parser extends EAtlas_metadata_viewer_record_abstract_parser {
	protected function parseXml($xml_str) {
		$extent_parser = new EAtlas_metadata_viewer_record_iso19139_extent_parser($this);

		$base_url = $this->getBaseURL();
		$uuid = $this->getUUID();

		// Parse the XML document
		$xml = new SimpleXMLElement($xml_str);
		$namespaces = $xml->getNamespaces(TRUE);
		$is_mcp_record = isset($namespaces['mcp']);

		// Some elements have a different namespace depending if it's a MCP record or not.
		// The document structure is mostly the same.
		$ns_MD_DataIdentification = $is_mcp_record ? 'mcp' : 'gmd';

		// All responsible parties found in the XML doc goes in here
		$responsibleParties = array();

		foreach ($xml->children('gmd', TRUE)->parentIdentifier as $xml_parentIdentifier) {
			foreach ($xml_parentIdentifier->children('gco', TRUE)->CharacterString as $xml_CharacterString) {
				if (!empty($xml_CharacterString)) {
					$this->addParentRecord((string)$xml_CharacterString);
				}
			}
		}

		foreach ($xml->children('gmd', TRUE)->identificationInfo as $xml_identificationInfo) {
			foreach ($xml_identificationInfo->children($ns_MD_DataIdentification, TRUE)->MD_DataIdentification as $xml_MD_DataIdentification) {
				foreach ($xml_MD_DataIdentification->children('gmd', TRUE)->citation as $xml_citation) {
					foreach ($xml_citation->children('gmd', TRUE)->CI_Citation as $xml_CI_Citation) {
						foreach ($xml_CI_Citation->children('gmd', TRUE)->title as $xml_title) {
							foreach ($xml_title->children('gco', TRUE)->CharacterString as $xml_CharacterString) {
								if (!empty($xml_CharacterString)) {
									// gmd:MD_Metadata > gmd:identificationInfo > gmd:MD_DataIdentification > gmd:citation > gmd:CI_Citation > gmd:title > gco:CharacterString
									$this->addTitle((string)$xml_CharacterString);
								}
							}
						}

						foreach ($xml_CI_Citation->children('gmd', TRUE)->citedResponsibleParty as $xml_citedResponsibleParty) {
							foreach ($xml_citedResponsibleParty->children('gmd', TRUE)->CI_ResponsibleParty as $xml_CI_ResponsibleParty) {
								$responsibleParties[] = $this->parseResponsibleParty($xml_CI_ResponsibleParty);
							}
						}
					}
				}

				foreach ($xml_MD_DataIdentification->children('gmd', TRUE)->pointOfContact as $xml_pointOfContact) {
					foreach ($xml_pointOfContact->children('gmd', TRUE)->CI_ResponsibleParty as $xml_CI_ResponsibleParty) {
						$responsibleParties[] = $this->parseResponsibleParty($xml_CI_ResponsibleParty);
					}
				}

				foreach ($xml_MD_DataIdentification->children('gmd', TRUE)->graphicOverview as $xml_graphicOverview) {
					foreach ($xml_graphicOverview->children('gmd', TRUE)->MD_BrowseGraphic as $xml_MD_BrowseGraphic) {
						$file_description = NULL;
						foreach ($xml_MD_BrowseGraphic->children('gmd', TRUE)->fileDescription as $xml_fileDescription) {
							foreach ($xml_fileDescription->children('gco', TRUE)->CharacterString as $xml_CharacterString) {
								if (!empty($xml_CharacterString)) {
									$file_description = (string)$xml_CharacterString;
								}
							}
						}

						foreach ($xml_MD_BrowseGraphic->children('gmd', TRUE)->fileName as $xml_fileName) {
							foreach ($xml_fileName->children('gco', TRUE)->CharacterString as $xml_CharacterString) {
								if (!empty($xml_CharacterString)) {
									// gmd:MD_Metadata > gmd:identificationInfo > gmd:MD_DataIdentification > gmd:graphicOverview > gmd:MD_BrowseGraphic > gmd:fileName > gco:CharacterString

									// Preview images are not regular:
									//   Anzlic           (a05f7892-eae4-7506-e044-00144fdd4fa6) => filename
									//                    (9793add6-c663-4bff-a2af-4f0d0f2a062b) => filename
									//   iso19139 MCP 1.5 (cb096f9c-1e72-40f6-a2e2-dbdfaed2517d) => filename
									//   iso19139 MCP 1.4 (f49c5d09-3c50-4d0c-b779-f2dd3642bfcf) => filename
									//   iso19139         (a86f062e-f47c-49f1-ace5-3e03a2272088) => Full URL
									//                    (f16e1b55-afc6-44fb-8eae-413fb91ff727) => Full URL

									$preview_image_url = (string)$xml_CharacterString;
									if (substr($preview_image_url, 0, 4) !== "http") {
										$preview_image_url = url(
											$base_url . 'resources.get',
											array(
												'query' => array(
													'uuid' => $uuid,
													'access' => 'public',
													'fname' => (string)$xml_CharacterString
												)
											)
										);
									}

									$this->addPreviewImage($preview_image_url, $file_description);
								}
							}
						}
					}
				}

				foreach ($xml_MD_DataIdentification->children('gmd', TRUE)->{'abstract'} as $xml_abstract) {
					foreach ($xml_abstract->children('gco', TRUE)->CharacterString as $xml_CharacterString) {
						if (!empty($xml_CharacterString)) {
							// gmd:MD_Metadata > gmd:identificationInfo > gmd:MD_DataIdentification > gmd:abstract > gco:CharacterString
							$this->addDesctiption((string)$xml_CharacterString);
						}
					}
				}

				foreach ($xml_MD_DataIdentification->children('gmd', TRUE)->purpose as $xml_purpose) {
					foreach ($xml_purpose->children('gco', TRUE)->CharacterString as $xml_CharacterString) {
						if (!empty($xml_CharacterString)) {
							// gmd:MD_Metadata > gmd:identificationInfo > gmd:MD_DataIdentification > gmd:purpose > gco:CharacterString
							$this->addPurpose((string)$xml_CharacterString);
						}
					}
				}

				foreach ($xml_MD_DataIdentification->children('gmd', TRUE)->supplementalInformation as $xml_supplementalInformation) {
					foreach ($xml_supplementalInformation->children('gco', TRUE)->CharacterString as $xml_CharacterString) {
						if (!empty($xml_CharacterString)) {
							// gmd:MD_Metadata > gmd:identificationInfo > gmd:MD_DataIdentification > gmd:supplementalInformation > gco:CharacterString
							$this->addSupplementalInformation((string)$xml_CharacterString);
						}
					}
				}

				// Resource constraints
				foreach ($xml_MD_DataIdentification->children('gmd', TRUE)->resourceConstraints as $xml_resourceConstraints) {
					foreach ($xml_resourceConstraints->children('gmd', TRUE)->MD_LegalConstraints as $xml_MD_LegalConstraints) {
						foreach ($xml_MD_LegalConstraints->children('gmd', TRUE)->useLimitation as $xml_useLimitation) {
							foreach ($xml_useLimitation->children('gco', TRUE)->CharacterString as $xml_CharacterString) {
								if (!empty($xml_CharacterString)) {
									$this->addUseLimitation((string)$xml_CharacterString);
								}
							}
						}
						foreach ($xml_MD_LegalConstraints->children('gmd', TRUE)->accessConstraints as $xml_accessConstraints) {
							foreach ($xml_accessConstraints->children('gco', TRUE)->CharacterString as $xml_CharacterString) {
								if (!empty($xml_CharacterString)) {
									$this->addAccessConstraint((string)$xml_CharacterString);
								}
							}
						}
						foreach ($xml_MD_LegalConstraints->children('gmd', TRUE)->useConstraints as $xml_useConstraints) {
							foreach ($xml_useConstraints->children('gco', TRUE)->CharacterString as $xml_CharacterString) {
								if (!empty($xml_CharacterString)) {
									$this->addUseConstraint((string)$xml_CharacterString);
								}
							}
						}
						foreach ($xml_MD_LegalConstraints->children('gmd', TRUE)->otherConstraints as $xml_otherConstraints) {
							foreach ($xml_otherConstraints->children('gco', TRUE)->CharacterString as $xml_CharacterString) {
								if (!empty($xml_CharacterString)) {
									$this->addOtherConstraint((string)$xml_CharacterString);
								}
							}
						}
					}

					foreach ($xml_resourceConstraints->children('gmd', TRUE)->MD_Constraints as $xml_MD_Constraints) {
						foreach ($xml_MD_Constraints->children('gmd', TRUE)->useLimitation as $xml_useLimitation) {
							foreach ($xml_useLimitation->children('gco', TRUE)->CharacterString as $xml_CharacterString) {
								if (!empty($xml_CharacterString)) {
									$this->addUseLimitation((string)$xml_CharacterString);
								}
							}
						}
					}

					foreach ($xml_resourceConstraints->children('gmd', TRUE)->MD_SecurityConstraints as $xml_MD_SecurityConstraints) {
						foreach ($xml_MD_SecurityConstraints->children('gmd', TRUE)->useLimitation as $xml_useLimitation) {
							foreach ($xml_useLimitation->children('gco', TRUE)->CharacterString as $xml_CharacterString) {
								if (!empty($xml_CharacterString)) {
									$this->addUseLimitation((string)$xml_CharacterString);
								}
							}
						}
					}

					foreach ($xml_resourceConstraints->children('mcp', TRUE)->MD_Commons as $xml_MD_Commons) {
						foreach ($xml_MD_Commons->children('mcp', TRUE)->licenseName as $xml_licenseName) {
							foreach ($xml_licenseName->children('gco', TRUE)->CharacterString as $xml_CharacterString) {
								if (!empty($xml_CharacterString)) {
									$this->addUseLimitation((string)$xml_CharacterString);
								}
							}
						}

						foreach ($xml_MD_Commons->children('mcp', TRUE)->otherConstraints as $xml_otherConstraints) {
							foreach ($xml_otherConstraints->children('gco', TRUE)->CharacterString as $xml_CharacterString) {
								if (!empty($xml_CharacterString)) {
									$this->addOtherConstraint((string)$xml_CharacterString);
								}
							}
						}
					}
				}

				foreach ($xml_MD_DataIdentification->children('gmd', TRUE)->descriptiveKeywords as $xml_descriptiveKeywords) {
					foreach ($xml_descriptiveKeywords->children('gmd', TRUE)->MD_Keywords as $xml_MD_Keywords) {
						foreach ($xml_MD_Keywords->children('gmd', TRUE)->keyword as $xml_keyword) {
							foreach ($xml_keyword->children('gco', TRUE)->CharacterString as $xml_CharacterString) {
								if (!empty($xml_CharacterString)) {
									// gmd:MD_Metadata > gmd:identificationInfo > gmd:MD_DataIdentification > gmd:descriptiveKeywords > gmd:MD_Keywords > gmd:keyword > gco:CharacterString
									$this->addKeyword((string)$xml_CharacterString);
								}
							}
						}
					}
				}

				foreach ($xml_MD_DataIdentification->children('gmd', TRUE)->extent as $xml_extent) {
					$extent_parser->parseExtent($xml_extent);
				}
			}
		}

		foreach ($xml->children('gmd', TRUE)->distributionInfo as $xml_distributionInfo) {
			foreach ($xml_distributionInfo->children('gmd', TRUE)->MD_Distribution as $xml_MD_Distribution) {
				foreach ($xml_MD_Distribution->children('gmd', TRUE)->transferOptions as $xml_transferOptions) {
					foreach ($xml_transferOptions->children('gmd', TRUE)->MD_DigitalTransferOptions as $xml_MD_DigitalTransferOptions) {
						foreach ($xml_MD_DigitalTransferOptions->children('gmd', TRUE)->onLine as $xml_onLine) {
							foreach ($xml_onLine->children('gmd', TRUE)->CI_OnlineResource as $xml_CI_OnlineResource) {
								$description = NULL;
								$name = NULL;
								$url = NULL;
								$type = NULL;
								foreach ($xml_CI_OnlineResource->children('gmd', TRUE)->description as $xml_description) {
									foreach ($xml_description->children('gco', TRUE)->CharacterString as $xml_CharacterString) {
										if (!empty($xml_CharacterString)) {
											// gmd:MD_Metadata > gmd:distributionInfo > gmd:MD_Distribution > gmd:transferOptions > gmd:MD_DigitalTransferOptions > gmd:onLine > gmd:CI_OnlineResource > gmd:description > gco:CharacterString
											$description = (string)$xml_CharacterString;
										}
									}
								}
								foreach ($xml_CI_OnlineResource->children('gmd', TRUE)->name as $xml_name) {
									foreach ($xml_name->children('gco', TRUE)->CharacterString as $xml_CharacterString) {
										if (!empty($xml_CharacterString)) {
											// gmd:MD_Metadata > gmd:distributionInfo > gmd:MD_Distribution > gmd:transferOptions > gmd:MD_DigitalTransferOptions > gmd:onLine > gmd:CI_OnlineResource > gmd:description > gco:CharacterString
											$name = (string)$xml_CharacterString;
										}
									}
								}
								foreach ($xml_CI_OnlineResource->children('gmd', TRUE)->linkage as $xml_linkage) {
									foreach ($xml_linkage->children('gmd', TRUE)->URL as $xml_URL) {
										if (!empty($xml_URL)) {
											// gmd:MD_Metadata > gmd:distributionInfo > gmd:MD_Distribution > gmd:transferOptions > gmd:MD_DigitalTransferOptions > gmd:onLine > gmd:CI_OnlineResource > gmd:linkage > gmd:URL
											$url = (string)$xml_URL;
										}
									}
								}
								foreach ($xml_CI_OnlineResource->children('gmd', TRUE)->protocol as $xml_protocol) {
									foreach ($xml_protocol->children('gco', TRUE)->CharacterString as $xml_CharacterString) {
										if (!empty($xml_CharacterString)) {
											// gmd:MD_Metadata > gmd:distributionInfo > gmd:MD_Distribution > gmd:transferOptions > gmd:MD_DigitalTransferOptions > gmd:onLine > gmd:CI_OnlineResource > gmd:protocol > gco:CharacterString
											$type = (string)$xml_CharacterString;
										}
									}
								}
								$this->addLink($url, $name, $description, $type);
							}
						}
					}
				}

				foreach ($xml_MD_Distribution->children('gmd', TRUE)->distributor as $xml_distributor) {
					foreach ($xml_distributor->children('gmd', TRUE)->MD_Distributor as $xml_MD_Distributor) {
						foreach ($xml_MD_Distributor->children('gmd', TRUE)->distributorContact as $xml_distributorContact) {
							foreach ($xml_distributorContact->children('gmd', TRUE)->CI_ResponsibleParty as $xml_CI_ResponsibleParty) {
								// gmd:distributionInfo > gmd:MD_Distribution > gmd:distributor > gmd:MD_Distributor > gmd:distributorContact > gmd:CI_ResponsibleParty
								$responsibleParties[] = $this->parseResponsibleParty($xml_CI_ResponsibleParty);
							}
						}
					}
				}
			}
		}

		foreach($responsibleParties as $responsibleParty) {
			$roles = isset($responsibleParty['roles']) ? $responsibleParty['roles'] : NULL;
			$names = isset($responsibleParty['individualNames']) ? $responsibleParty['individualNames'] : NULL;
			$positions = isset($responsibleParty['positionNames']) ? $responsibleParty['positionNames'] : NULL;
			$organisations = isset($responsibleParty['organisationNames']) ? $responsibleParty['organisationNames'] : NULL;
			$emails = isset($responsibleParty['electronicMailAddress']) ? $responsibleParty['electronicMailAddress'] : NULL;

			$unknown_role = TRUE;
			if (!empty($roles)) {
				if (in_array('author', $roles)) {
					$unknown_role = FALSE;
					$this->addAuthor($names, $positions, $organisations, $emails);
				}
				if (in_array('owner', $roles)) {
					$unknown_role = FALSE;
					$this->addOwner($names, $positions, $organisations, $emails);
				}
				if (in_array('distributor', $roles)) {
					$unknown_role = FALSE;
					$this->addDistributor($names, $positions, $organisations, $emails);
				}
				if (in_array('custodian', $roles)) {
					$unknown_role = FALSE;
					$this->addCustodian($names, $positions, $organisations, $emails);
				}
				if (in_array('pointofcontact', $roles)) {
					$unknown_role = FALSE;
					$this->addPointOfContact($names, $positions, $organisations, $emails);
				}
				if (in_array('metadatacontact', $roles)) {
					$unknown_role = FALSE;
					$this->addMetadataContact($names, $positions, $organisations, $emails);
				}
				if (in_array('principalinvestigator', $roles)) {
					$unknown_role = FALSE;
					$this->addPrincipalInvestigator($names, $positions, $organisations, $emails);
				}
				if (in_array('coinvestigator', $roles)) {
					$unknown_role = FALSE;
					$this->addCoInvestigator($names, $positions, $organisations, $emails);
				}
			}
			// Default
			if ($unknown_role) {
				$this->addPointOfContact($names, $positions, $organisations, $emails);
			}
		}
	}

	private function parseResponsibleParty($xml_CI_ResponsibleParty) {
		$party = array();

		// roles
		foreach ($xml_CI_ResponsibleParty->children('gmd', TRUE)->role as $xml_role) {
			foreach ($xml_role->children('gmd', TRUE)->CI_RoleCode as $xml_CI_RoleCode) {
				$role = strtolower((string)$xml_CI_RoleCode->attributes()->codeListValue);
				if (!empty($role)) {
					if (!isset($party['roles'])) {
						$party['roles'] = array();
					}
					$party['roles'][] = $role;
				}
			}
		}

		// individualNames
		foreach ($xml_CI_ResponsibleParty->children('gmd', TRUE)->individualName as $xml_individualName) {
			foreach ($xml_individualName->children('gco', TRUE)->CharacterString as $xml_CharacterString) {
				if (!empty($xml_CharacterString)) {
					if (!isset($party['individualNames'])) {
						$party['individualNames'] = array();
					}
					$party['individualNames'][] = (string)$xml_CharacterString;
				}
			}
		}

		// positionNames
		foreach ($xml_CI_ResponsibleParty->children('gmd', TRUE)->positionName as $xml_positionName) {
			foreach ($xml_positionName->children('gco', TRUE)->CharacterString as $xml_CharacterString) {
				if (!empty($xml_CharacterString)) {
					if (!isset($party['positionNames'])) {
						$party['positionNames'] = array();
					}
					$party['positionNames'][] = (string)$xml_CharacterString;
				}
			}
		}

		// organisationNames
		foreach ($xml_CI_ResponsibleParty->children('gmd', TRUE)->organisationName as $xml_organisationName) {
			foreach ($xml_organisationName->children('gco', TRUE)->CharacterString as $xml_CharacterString) {
				if (!empty($xml_CharacterString)) {
					if (!isset($party['organisationNames'])) {
						$party['organisationNames'] = array();
					}
					$party['organisationNames'][] = (string)$xml_CharacterString;
				}
			}
		}

		// electronicMailAddress
		foreach ($xml_CI_ResponsibleParty->children('gmd', TRUE)->contactInfo as $xml_contactInfo) {
			foreach ($xml_contactInfo->children('gmd', TRUE)->CI_Contact as $xml_CI_Contact) {
				foreach ($xml_CI_Contact->children('gmd', TRUE)->address as $xml_address) {
					foreach ($xml_address->children('gmd', TRUE)->CI_Address as $xml_CI_Address) {
						foreach ($xml_CI_Address->children('gmd', TRUE)->electronicMailAddress as $xml_electronicMailAddress) {
							foreach ($xml_electronicMailAddress->children('gco', TRUE)->CharacterString as $xml_CharacterString) {
								if (!empty($xml_CharacterString)) {
									if (!isset($party['electronicMailAddress'])) {
										$party['electronicMailAddress'] = array();
									}
									$party['electronicMailAddress'][] = (string)$xml_CharacterString;
								}
							}
						}
					}
				}
			}
		}

		return $party;
	}

	private function getResponsiblePartyMergedValue($responsibleParty, $key) {
		if (!isset($responsibleParty[$key]) || empty($responsibleParty[$key])) {
			return NULL;
		}
		return implode(', ', $responsibleParty[$key]);
	}

	// Get links matching certains Protocols (type)
	public function getWmsLinks() {
		return $this->getLinksByTypes(
			array(EATLAS_METADATA_WMS_GM_LINK_PROTOCOL)
		);
	}
	public function getKmlLinks() {
		return $this->getLinksByTypes(
			array(EATLAS_METADATA_KML_LINK_PROTOCOL)
		);
	}
	public function getRelatedLinks() {
		return $this->getLinksByTypes(
			array(EATLAS_METADATA_RELATED_PROTOCOL)
		);
	}
	public function getPublicationLinks() {
		return $this->getLinksByTypes(
			array(EATLAS_METADATA_WMS_GM_LINK_PROTOCOL, EATLAS_METADATA_RELATED_PROTOCOL, EATLAS_METADATA_POINT_OF_TRUTH),
			TRUE
		);
	}
}

?>
