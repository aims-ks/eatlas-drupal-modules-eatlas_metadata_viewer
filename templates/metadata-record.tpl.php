<?php
$preview_images = $record->getPreviewImages();

// Find the thumbnail image
$preview_image = NULL;
$preview_image_style = '';
if (!empty($preview_images)) {
	foreach($preview_images as $_preview_image) {
		if (strtolower($_preview_image['description']) === 'thumbnail') {
			$preview_image = $_preview_image;
		}
	}
	if ($preview_image === NULL) {
		$preview_image = $preview_images[0];
	}

	$preview_image_style = '';
	if ($preview_image['width']) {
		$preview_image_style .= 'width:' . $preview_image['width'] . 'px;';
	}
	if ($preview_image['height']) {
		$preview_image_style .= 'height:' . $preview_image['height'] . 'px;';
	}
}

$desctiption = NULL;
$desctiptions = $record->getDesctiptions();
if (!empty($desctiptions)) {
	$desctiption = nl2br(check_plain(trim(implode("\n\n", $desctiptions))));
}

$purpose = NULL;
$purposes = $record->getPurposes();
if (!empty($purposes)) {
	$purpose = nl2br(check_plain(trim(implode("\n\n", $purposes))));
}

$supplemental_information = NULL;
$supplemental_informations = $record->getSupplementalInformations();
if (!empty($supplemental_informations)) {
	$supplemental_information = nl2br(check_plain(trim(implode("\n\n", $supplemental_informations))));
}


$publication_links = $record->getPublicationLinks();
$publication_links_count = count($publication_links);

$related_links = $record->getRelatedLinks();
$related_links_count = count($related_links);


// Parents / Children

$parent_records = $record->getParentRecords();
$parent_records_count = count($parent_records);

// Sent to the template from eatlas_metadata_viewer_record.inc
$child_records_count = count($child_records);


// Temporal extents

$temporal_extents = $record->getTemporalExtents();
$temporal_extents_count = count($temporal_extents);


// Responsible parties

$authors = $record->getAuthors();
$authors_count = count($authors);

$custodians = $record->getCustodians();
$custodians_count = count($custodians);

$points_of_contact = $record->getPointsOfContact();
$points_of_contact_count = count($points_of_contact);

$owners = $record->getOwners();
$owners_count = count($owners);

$distributors = $record->getDistributors();
$distributors_count = count($distributors);

$metadata_contacts = $record->getMetadataContacts();
$metadata_contacts_count = count($metadata_contacts);

$principal_investigators = $record->getPrincipalInvestigators();
$principal_investigators_count = count($principal_investigators);

$co_investigators = $record->getCoInvestigators();
$co_investigators_count = count($co_investigators);


// Resource constraints

$use_limitations = $record->getUseLimitations();
$access_constraints = $record->getAccessConstraints();
$use_constraints = $record->getUseConstraints();
$other_constraints = $record->getOtherConstraints();
$resource_constraints_count = count($use_limitations) + count($access_constraints) + count($use_constraints) + count($other_constraints);

$keywords = $record->getKeywords();
$keywords_count = count($keywords);
?>

<div class="metadata-record<?php print ($record->isPublished() ? '' : ' metadata-record-unpublished'); ?>">
	<div class="map">
		<div id="map"></div>

		<?php if ($temporal_extents_count): ?>
			<span class="label"><?php print $temporal_extents_count > 1 ? t('Data collection dates') : t('Data collection date'); ?></span>
			<ul class="collection-dates">
				<?php print eatlas_metadata_viewer_render_temporal_extents($temporal_extents); ?>
			</ul>
		<?php endif; ?>
	</div>

	<?php if ($preview_image !== NULL): ?>
		<div class="preview-image">
			<img alt="<?php print check_plain($preview_image['description']); ?>" src="<?php print $preview_image['url']; ?>" style="<?php print $preview_image_style; ?>" />
		</div>
	<?php endif; ?>

	<div class="desctiption">
		<p><?php print $desctiption; ?></p>
	</div>

	<div class="purpose">
		<p><?php print $purpose; ?></p>
	</div>

	<div class="supplemental_information">
		<p><?php print $supplemental_information; ?></p>
	</div>

	<div class="metadata">
		<div class="identification-info">
			<?php if ($principal_investigators_count): ?>
				<div class="responsible-parties">
					<span class="label"><?php print $principal_investigators_count > 1 ? t('Principal investigators') : t('Principal investigator'); ?></span>
					<ul class="principal-investigators-list">
						<?php print eatlas_metadata_viewer_render_responsible_parties($principal_investigators); ?>
					</ul>
				</div>
			<?php endif; ?>

			<?php if ($co_investigators_count): ?>
				<div class="responsible-parties">
					<span class="label"><?php print $co_investigators_count > 1 ? t('Co investigators') : t('Co investigator'); ?></span>
					<ul class="co-investigators-list">
						<?php print eatlas_metadata_viewer_render_responsible_parties($co_investigators); ?>
					</ul>
				</div>
			<?php endif; ?>

			<?php if ($authors_count): ?>
				<div class="responsible-parties">
					<span class="label"><?php print $authors_count > 1 ? t('Authors') : t('Author'); ?></span>
					<ul class="authors-list">
						<?php print eatlas_metadata_viewer_render_responsible_parties($authors); ?>
					</ul>
				</div>
			<?php endif; ?>

			<?php if ($custodians_count): ?>
				<div class="responsible-parties">
					<span class="label"><?php print $custodians_count > 1 ? t('Custodians') : t('Custodian'); ?></span>
					<ul class="custodians-list">
						<?php print eatlas_metadata_viewer_render_responsible_parties($custodians); ?>
					</ul>
				</div>
			<?php endif; ?>

			<?php if ($points_of_contact_count): ?>
				<div class="responsible-parties">
					<span class="label"><?php print $points_of_contact_count > 1 ? t('Points of contact') : t('Point of contact'); ?></span>
					<ul class="points-of-contact-list">
						<?php print eatlas_metadata_viewer_render_responsible_parties($points_of_contact); ?>
					</ul>
				</div>
			<?php endif; ?>

			<?php if ($owners_count): ?>
				<div class="responsible-parties">
					<span class="label"><?php print $owners_count > 1 ? t('Data owners') : t('Data owner'); ?></span>
					<ul class="owners-list">
						<?php print eatlas_metadata_viewer_render_responsible_parties($owners); ?>
					</ul>
				</div>
			<?php endif; ?>

			<?php if ($distributors_count): ?>
				<div class="responsible-parties">
					<span class="label"><?php print $distributors_count > 1 ? t('Distributors') : t('Distributor'); ?></span>
					<ul class="distributors-list">
						<?php print eatlas_metadata_viewer_render_responsible_parties($distributors); ?>
					</ul>
				</div>
			<?php endif; ?>

			<?php if ($metadata_contacts_count): ?>
				<div class="responsible-parties">
					<span class="label"><?php print t('Metadata contact'); ?></span>
					<ul class="metadata-contacts-list">
						<?php print eatlas_metadata_viewer_render_responsible_parties($metadata_contacts); ?>
					</ul>
				</div>
			<?php endif; ?>
		</div>


		<div class="distribution-info">
			<?php if ($parent_records_count): ?>
				<div class="related-links">
					<span class="label"><?php print $parent_records_count > 1 ? t('Parent records') : t('Parent record'); ?></span>
					<ul class="parent-records-list">
						<?php print eatlas_metadata_viewer_render_parent_records($parent_records); ?>
					</ul>
				</div>
			<?php endif; ?>

			<?php if ($publication_links_count): ?>
				<div class="related-links">
					<span class="label"><?php print $publication_links_count > 1 ? t('Publications / Data') : t('Publication / Data'); ?></span>
					<ul class="publication-links-list">
						<?php print eatlas_metadata_viewer_render_links($publication_links); ?>
					</ul>
				</div>
			<?php endif; ?>

			<?php if ($related_links_count): ?>
				<div class="related-links">
					<span class="label"><?php print $related_links_count > 1 ? t('Related Websites / Services') : t('Related Website / Service'); ?></span>
					<ul class="related-links-list">
						<?php print eatlas_metadata_viewer_render_links($related_links); ?>
					</ul>
				</div>
			<?php endif; ?>

			<?php if ($child_records_count): ?>
				<div class="related-links">
					<span class="label"><?php print $child_records_count > 1 ? t('Child records') : t('Child record'); ?></span>
					<ul class="child-records-list">
						<?php print eatlas_metadata_viewer_render_child_records($child_records); ?>
					</ul>
				</div>
			<?php endif; ?>

			<?php if ($resource_constraints_count): ?>
				<div class="resource-constraints">
					<span class="label"><?php print $resource_constraints_count > 1 ? t('Data Usage Constraints') : t('Data Usage Constraint'); ?></span>
					<ul class="resource-constraints-list">
						<?php print eatlas_metadata_viewer_render_resource_constraints($use_limitations); ?>
						<?php print eatlas_metadata_viewer_render_resource_constraints($access_constraints); ?>
						<?php print eatlas_metadata_viewer_render_resource_constraints($use_constraints); ?>
						<?php print eatlas_metadata_viewer_render_resource_constraints($other_constraints); ?>
					</ul>
				</div>
			<?php endif; ?>
		</div>
	</div>

	<div class="buttons">
		<a href="<?php print $record->getRecordUrl(); ?>" target="_blank"><span class="button"><?php print t('Full Metadata Record'); ?></span></a>
	</div>

	<?php if ($keywords_count): ?>
		<div class="keywords">
			<span class="label"><?php print $keywords_count > 1 ? t('Tags') : t('Tag'); ?></span>
			<ul class="keywords-list">
				<?php print eatlas_metadata_viewer_render_keywords($keywords); ?>
			</ul>
		</div>
	<?php endif; ?>
</div>
