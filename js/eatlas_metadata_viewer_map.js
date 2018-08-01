// NOTE: eatlas_metadata_viewer is defined in metadata-record.tpl.php
(function ($) {

	// NOTE: The Leaflet API has a latLngBounds object, but it CAN'T
	//   be initialised without 2 points. It's possible to create a
	//   wrapper around latLngBounds.extend, and initialise it on the
	//   first call, but the following solution is more strait forward.
	function _extendBounds(bounds, point) {
		if (bounds['north'] === null || point[0] > bounds['north']) {
			bounds['north'] = point[0];
		}
		if (bounds['south'] === null || point[0] < bounds['south']) {
			bounds['south'] = point[0];
		}

		if (bounds['east'] === null || point[1] > bounds['east']) {
			bounds['east'] = point[1];
		}
		if (bounds['west'] === null || point[1] < bounds['west']) {
			bounds['west'] = point[1];
		}
	}

	/*
	TESTS:
		(MCP) KML + WMS + POINTS: a5a02dc8-16b4-4b50-abad-af4a1c1e9c49
		(ANZLIC) BBOX + WMS: 9793add6-c663-4bff-a2af-4f0d0f2a062b
		(MCP) BBOXES + POLYGON: bbeba305-16f1-4008-9e32-970e7d1c19b7
		(ERROR): 4ab840c6-da0f-41b5-840e-d74b8e48fe0e
	*/

	// Execute when the page is ready
	$(document).ready(function(){
		if (Drupal.settings.eatlas_metadata_viewer) {
			// Load the map - TODO Load OUR map instead of this demo (or get a Access token!)
			// http://leafletjs.com/reference.html#tilelayer-wms
			var map_centre_lat = parseFloat(Drupal.settings.eatlas_metadata_viewer.map_centre_lat);
			var map_centre_lon = parseFloat(Drupal.settings.eatlas_metadata_viewer.map_centre_lon);
			var map_zoom = parseInt(Drupal.settings.eatlas_metadata_viewer.map_zoom);
			var map = L.map('map').setView([map_centre_lat, map_centre_lon], map_zoom);

			// Load the background layer
			var map_wms_server = Drupal.settings.eatlas_metadata_viewer.map_wms_server;
			var map_wms_layer = Drupal.settings.eatlas_metadata_viewer.map_wms_layer;
			if (map_wms_server && map_wms_layer) {
				L.tileLayer.wms(map_wms_server, {
					layers: map_wms_layer
				}).addTo(map);
			}

			// Load record's WMS layers
			var wms_links = Drupal.settings.eatlas_metadata_viewer.wms_links;
			if (wms_links && wms_links.length > 0) {
				for (var i=0; i<wms_links.length; i++) {
					var wms_server = wms_links[i]['url'];
					var wms_layer = wms_links[i]['name'];
					if (wms_server && wms_layer) {
						L.tileLayer.wms(wms_server, {
							layers: wms_layer,
							format: 'image/png',
							transparent: true
						}).addTo(map);
					}
				}
			}

			// Load record's KML layers
			// KML library can be downloaded from:
			//   https://github.com/shramov/leaflet-plugins/tree/master/layer/tile
			// IMPORTANT: KML only works on the same domain
			//   (cross origin protection in browser)
			// NOTE: KML are not common enough in records to justify the bandwidth
			//   overhead of the plugin.
			/*
			if (L.KML) {
				var kml_links = Drupal.settings.eatlas_metadata_viewer.kml_links;
				if (kml_links && kml_links.length > 0) {
					for (var i=0; i<wms_links.length; i++) {
						var kmlLayer = new L.KML(kml_links[i]['url'], {async: true});
						//kmlLayer.on("loaded", function(e) { 
						//	map.fitBounds(e.target.getBounds());
						//});
						map.addLayer(kmlLayer);
					}
				}
			}
			*/

			if (Drupal.settings.eatlas_metadata_viewer) {
				var json_polygons = Drupal.settings.eatlas_metadata_viewer.polygons;
				var json_points = Drupal.settings.eatlas_metadata_viewer.points;
				var json_bboxes = Drupal.settings.eatlas_metadata_viewer.bboxes;

				var bounds = {
					'north': null,
					'south': null,
					'east': null,
					'west': null
				};

				if (json_polygons) {
					for(var i=0; i<json_polygons.length; i++) {
						var json_polygon_el = json_polygons[i];
						var json_polygon = json_polygon_el['polygon'];
						var json_description = json_polygon_el['description'];

						var pointArray = [];
						for(var j=0; j<json_polygon.length; j++) {
							var json_point = json_polygon[j];

							// The JSON object created by Drupal only contains String values.
							// Leaflet needs numbers.
							var lat = parseFloat(json_point['lat']),
								lon = parseFloat(json_point['lon']);

							var point = [lat, lon];
							_extendBounds(bounds, point);

							pointArray.push(point);
						}

						var l_polygon = L.polygon(pointArray, { fillOpacity: 0 });
						l_polygon.addTo(map);
						if (json_description) {
							l_polygon.bindPopup(json_description);
						}
					}
				}

				if (json_points) {
					for(var i=0; i<json_points.length; i++) {
						var json_point_el = json_points[i];
						var json_point = json_point_el['point'];
						var json_description = json_point_el['description'];

						var lat = parseFloat(json_point['lat']),
							lon = parseFloat(json_point['lon']);

						var point = [lat, lon];
						_extendBounds(bounds, point);

						var l_point = L.marker(point);
						l_point.addTo(map);
						if (json_description) {
							l_point.bindPopup(json_description);
						}
					}
				}

				if (json_bboxes) {
					for(var i=0; i<json_bboxes.length; i++) {
						var json_bbox_el = json_bboxes[i];
						var json_bbox = json_bbox_el['bbox'];
						var json_description = json_bbox_el['description'];

						// The JSON object created by Drupal only contains String values.
						// Leaflet needs numbers.
						var north = parseFloat(json_bbox['north']),
							south = parseFloat(json_bbox['south']),
							east = parseFloat(json_bbox['east']),
							west = parseFloat(json_bbox['west']);

						var pointArray = [
							[north, west],
							[north, east],
							[south, east],
							[south, west],
							[north, west] // Close the polygon
						];
						_extendBounds(bounds, [north, west]);
						_extendBounds(bounds, [south, east]);

						var l_bbox = L.polygon(pointArray, { fillOpacity: 0 });
						l_bbox.addTo(map);
						if (json_description) {
							l_bbox.bindPopup(json_description);
						}
					}
				}

				var southWest = L.latLng(bounds['south'], bounds['west']),
					northEast = L.latLng(bounds['north'], bounds['east']),
					latlngbounds = L.latLngBounds(southWest, northEast);

				map.fitBounds(latlngbounds);
			}
		}
	});
})(jQuery);
