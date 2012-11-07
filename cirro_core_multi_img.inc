<?php
/**
 * @file
 *   All pieces of the multi-img system accept for the schema and entity_info.
 */

/**
 * Build multi_img forms.
 *
 * @param array $multi_imgs
 *   multi_img array from entity info
 * @param $type
 *   Entity type
 * @param $id
 *   Item ID
 *
 * @return
 *   Full markup for multi_img forms or placeholders.
 */
function cirro_core_multi_img_build_forms($item, $multi_imgs, $type, $id) {
	
	// Init
	$imgs = '';

	// Check for add
	if ( $item === 'add' ) {

		// Loop through multi-imgs
		foreach ( $multi_imgs as $name => $info ) {

			// Build multi img add placeholder
			$img_form = "
				<div class=\"multi-img-form-wrapper\">
					<div class=\"title-wrapper\">
						<h2>{$info['label']}</h2>
					</div>
					<div class=\"note-wrapper\">
						<h4>Finish creating item in order to add images</h4>
					</div>
				</div>
			";

			// Add to imgs markup
			$imgs .= $img_form;

		}

	} else {

		// Loop through multi-imgs
		foreach ( $multi_imgs as $field => $info ) {

			// Query existing imgs
			$query_string = "
				SELECT id, fid, weight, img_alt, img_title
					FROM cirro_multi_img
					WHERE parent_id = :id
					AND parent_type = :type
					AND parent_field = :field
					ORDER BY weight, created
			";
		
			$placeholders = array(
				':id' => $id,
				':type' => $type,
				':field' => $field
			);
		
			$query = db_query($query_string, $placeholders);
		
			$imgs = $query->fetchAll();
			
			// Count imgs
			$img_count = count($imgs);

			// Build img weight form options
			$img_weight_options = array(
				'parent_id' => $id,
				'parent_type' => $type,
				'parent_field' => $field,
				'imgs' => $imgs,
			);

			// Get img weight form
			$weight_form_array = drupal_get_form(
				"cirro_core_{$type}_{$field}_multi_img_weights", $img_weight_options);

			if ( isset($info['limit']) && $img_count >= $info['limit'] ) {
				
				// Replace upload field with limit warning
				$upload_form = "
					<div class=\"alert alert-info\">
						You have reached the upload limit of {$info['limit']} images.<br />
						Delete some images in order to add more.
					</div>
				";
				
			} else {

				// Load forms
				$upload_form_array = drupal_get_form(
					"cirro_core_{$type}_{$field}_multi_img_upload",
					$id,
					$type,
					$field
				);
				
				// Render upload form
				$upload_form = drupal_render($upload_form_array);
				
			}

			// Render forms
			$weight_form = drupal_render($weight_form_array);

			// Build multi img form elements
			$img_form = "
				<div class=\"multi-img-form-wrapper box-white\">
					<div class=\"title-wrapper\">
						<h2>{$info['label']}</h2>
					</div>
					<div class=\"weight-wrapper\" />
						$weight_form
					</div>
					<div class=\"line-seperator\"></div>
					<div class=\"upload-wrapper\">
						<h4>Upload Image</h4>
						$upload_form
					</div>
				</div>
			";

			// Add to imgs markup
			$imgs .= $img_form;

		}

	}
	
	return $imgs;
	
}

/**
 * Multi Img weight form.
 *
 * @parm $id
 *   Parent ID
 * @param $type
 *   Entity type
 * @param $field
 *   Entity field name
 * @param $imgs
 *   Array of img items
 */
function cirro_core_multi_img_weight_form($form, &$form_state, $options) {

	// Init
	$form = array();
	$id = $options['parent_id'];
	$type = $options['parent_type'];
	$field = $options['parent_field'];
	$imgs = $options['imgs'];

	// Build weight options
	$weight_options = array();

	for ( $i = -40; $i <= 40; $i++ ) {

		$weight_options[$i] = $i;

	}

	// Add parent info to form
	$form['parent_id'] = array(
		'#type' => 'hidden',
		'#value' => $id,
	);

	$form['parent_type'] = array(
		'#type' => 'hidden',
		'#value' => $type,
	);

	$form['parent_field'] = array(
		'#type' => 'hidden',
		'#value' => $field,
	);

	// Setup imgs array
	$form['imgs'] = array();
	$form['imgs']['#tree'] = TRUE;

	// Loop through images
	foreach ( $imgs as $img ) {

		// Check for fid of 0
		if ( $img->fid == 0 ) {

			// Delete multi_img
			entity_get_controller('multi_img')->delete($img->id);
			
			// Continue on in loop
			continue;
			
		}

		// Load file object
		$file = file_load($img->fid);

		// Load file info
		$file_name = $file->filename;
		$file_uri = $file->uri;
		$file_path = file_create_url($file_uri);

	  $img_styled = '';

	  // Load image vars
	  $img_vars = array(
	  	'style_name' => 'thumbnail',
	  	'path' => $file_uri,
	  	'alt' => $file_name,
	  	'title' => $file_name,
	  	'width' => NULL,
	  	'height' => NULL,
	  );

	  // Theme/style image
	  $img_styled =  theme_image_style($img_vars);

		// Init fields array
		$fields = array();

		$fields['id'] = array(
			'#type' => 'hidden',
			'#value' => $img->id,
		);

		$fields['thumbnail'] = array(
			'#markup' => l(
				$img_styled,
				$file_path,
				array(
					'html' => TRUE,
					'attributes' => array('target' => '_blank'),
				)
			),
		);

		$fields['weight'] = array(
			'#type' => 'select',
			'#title' => 'Weight',
			'#default_value' => $img->weight,
			'#options' => $weight_options,
		);

		// Add fields
		$fields['img_title'] = array(
			'#type' => 'textfield',
			'#attributes' => array(
				'placeholder' => 'Title Text',
				'title' => 'Title Text',
			),
			'#default_value' => $img->img_title,
		);

		$fields['img_alt'] = array(
			'#type' => 'textfield',
			'#attributes' => array(
				'placeholder' => 'Alt Text',
				'title' => 'Alt Text',
			),
			'#default_value' => $img->img_alt,
		);

		$fields['delete_link'] = array(
			'#markup' => l(
				'<i class="icon-remove"></i> ',
				"admin/dash/multi_img/{$img->id}/delete",
				array(
					'html' => TRUE,
					'attributes' => array(
						'class' => array('btn', 'btn-small'),
						'title' => 'Remove Image',
					),
					'query' => array(
						'destination' => "admin/dash/$type/$id/edit"
					),
				)
			),
		);

		// Add onto imgs index
		$form['imgs'][$img->id] = $fields;

	}

	if ( !empty($imgs) ) {
	
		// Add submit
		$form['submit'] = array(
			'#type' => 'submit',
			'#value' => 'Update Images',
		);
		
	}
	
	return $form;
	
}

/**
 * Multi Img weights submit
 */
function cirro_core_multi_img_weight_form_submit($form, &$form_state) {

	// Init
	$values = $form_state['values'];

	$objects = array();

	// Loop through imgs
	foreach ( $values['imgs'] as $id => $data ) {

		// Cast fields object
		$fields_obj = (object) $data;

		// Update img
		entity_get_controller('multi_img')->update($fields_obj);

	}

	// Display success message
	drupal_set_message('Images Updated');

}

/**
 * Multi Image File upload form
 *
 * @param $id
 *   Parent entity id
 * @param $type
 *   Parent entity type
 * @param $field
 *   Parent entity field name
 */
function cirro_core_multi_img_upload_form($form, &$form_state, $id, $type, $field) {

	// Init
	$form = array();

	// Parent id
	$form['parent_id'] = array(
		'#type' => 'hidden',
		'#value' => $id,
	);

	// Type field
	$form['parent_type'] = array(
		'#type' => 'hidden',
		'#value' => $type,
	);

	// Field name
	$form['parent_field'] = array(
		'#type' => 'hidden',
		'#value' => $field,
	);
	
	// Weight
	$form['weight'] = array(
		'#type' => 'hidden',
		'#value' => 40,
	);

	// File field
	$form['fid'] = array(
		'#type' => 'managed_file',
		'#upload_location' => "public://multi_imgs/$type/$field",
		'#upload_validators' => array(
			'file_validate_extensions' => array('jpg jpeg gif png'),
			'file_validate_size' => array(1048576),
		),
	);

	// Submit
	$form['submit'] = array(
		'#type' => 'submit',
		'#value' => 'Save Img',
	);

	return $form;
}

/**
 * Multi Image File Upload submit.
 */
function cirro_core_multi_img_upload_form_submit($form, &$form_state) {

	// Init
	$fields = $form_state['values'];


	// Cast fields obj
	$fields_obj = (object) $fields;

	// Attempt item create
	if ( ( $item = entity_get_controller('multi_img')->create($fields_obj) ) !== FALSE ) {

		drupal_set_message('Image Created');

	} else {

		drupal_set_message('Image creation failure', 'error');

	}

}

/**
 * Theme multi image weights forms.
 *
 * @param $variables
 *   Themeing variables array
 */
function theme_cirro_core_multi_img_weight_form($variables) {
	
	// Init
	$output = '';
	$rows = array();
	
	// Load form array
	$form = $variables['form'];
	
	// Activate tabledrag shiz
	drupal_add_tabledrag('draggable-table', 'order', 'sibling', 'weight-group');
	
	// Loop through form Elements
	foreach ( element_children($form['imgs']) as $key ) {
		
		// Init
		$element = array();
		$data = '';
		$row = array();
		
		// Skip none numeric keys
		if ( !is_numeric($key) ) continue;
		
		// Load element
		$element = &$form['imgs'][$key];
		
		// Add class to weight field
		$element['weight']['#attributes']['class'] = array('weight-group');
		
		// Put data fields together
		$data .= drupal_render($element['id']);
		$data .= drupal_render($element['img_title']);
		$data .= drupal_render($element['img_alt']);
		
		// Build row
		$row = array();
		$row[] = drupal_render($element['thumbnail']);
		$row[] = $data;
		$row[] = drupal_render($element['weight']);
		$row[] = drupal_render($element['delete_link']);
		
		// Add to rows
		$rows[] = array( 'data' => $row, 'class' => array('draggable'));

	}
	
	// Define headers
	$headers = array(
		'File',
		'Data',
		'Weight',
		'',
	);

	// Define table vars
	$table_vars = array(
		'header' => $headers,
		'rows' => $rows,
		'attributes' => array(
			'id' => 'draggable-table',
			'class' => array(
				'table',
				'table-bordered',
				'table-striped'
			),
		),
		'caption' => 'Drag and Drop to change order',
		'colgroups' => array(),
		'sticky' => FALSE,
		'empty' => 'No Images found',
	);
	
	// Add table to output
	$output = theme_table($table_vars);
	
	// Render remaining form elements
	$output .= drupal_render_children($form);	
	
	return $output;

}

/**
 * Multi-image load helper function
 */
function cirro_core_multi_img_load_helper($type, $id, $field){
 
 // Init stuff here
 $imgs = array();
 
	
	// Query items
	$img_query_string = "
	SELECT m.img_alt, m.img_title, f.uri, f.fid 
  	FROM cirro_multi_img AS m
  	JOIN file_managed AS f
  		ON m.fid = f.fid
  	WHERE parent_id = :id
  	AND parent_type = :type
  	AND parent_field = :field
  	ORDER BY m.weight
	";
	
	$placeholders = array(
		':id' => $id,
		':type' => $type,
		':field' => $field,
	);
	
		$img_query = db_query($img_query_string, $placeholders);
		$img_results = $img_query->fetchAll();
 
 // If images return url, title, alt
 foreach ( $img_results as $img_result ) {
	 
	 $img_result->url = file_create_url($img_result->uri);
	 
	 $imgs[] = $img_result;

 }

 return $imgs;	 
}