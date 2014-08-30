<?php

/**
 * @file
 * Admin page callbacks for the block module.
 */

/**
 * Theme the administer buttons page.
 *
 * @ingroup themeable
 */
function theme_sharebar_buttons_table($variables) {
  $out = '';
  $out .= '<div>' . l('Add New Button', 'admin/settings/sharebar/add') . '</div>';
  $buttons = unserialize(variable_get('sharebar_buttons', sharebar_buttons_def()));
  if (is_array($buttons) && count($buttons)) {
    usort($buttons, "sharebar_cmp_up");
    foreach ($buttons as $key => $value) {
      $row = array();
      $row[] = array('data' => ($value->enabled ? t('Yes') : t('No')));
      $row[] = array('data' => $value->name);
      $row[] = array('data' => $value->big_button);
      $row[] = array('data' => $value->small_button);
      $row[] = array('data' => $value->weight);
      $row[] = array(
        'data' => l('Edit', 'admin/settings/sharebar/edit/' . $value->machine_name) .
        ' | ' .
        l('Delete', 'admin/settings/sharebar/del/' . $value->machine_name));
      $rows[] = $row;
    }
  }
  if (count($rows)) {
    $header = array(
      t('Enabled'),
      t('Name'),
      t('Big button'),
      t('Small button'),
      t('Weight'),
      t('Actions'),
    );
    $out .= theme('table', $header, $rows);
  }
  else {
    $out .= '<b>' . t('No data') . '</b>';
  }

  return $out;
}

/**
 * Form builder: create buttons.
 */
function sharebar_addbutton(&$form_state, $edit = array()) {
  $mname = arg(4);
  $button = new stdClass();
  $button->name = $button->machine_name = $button->big_button = $button->small_button = $button->enabled = $button->weight = '';
  if ($mname) {
    $buttons = unserialize(variable_get('sharebar_buttons', sharebar_buttons_def()));
    if (is_array($buttons) && count($buttons)) {
      $button = $buttons[$mname];
    }
  }

  $form['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#default_value' => $button->name,
    '#maxlength' => 255,
    '#required' => TRUE,
  );

  $form['old_machine_name'] = array(
    '#type' => 'value',
    '#value' => $button->machine_name,
  );

  $form['big_button'] = array(
    '#type' => 'textarea',
    '#title' => t('Big button'),
    '#default_value' => $button->big_button,
    '#required' => TRUE,
  );

  $form['small_button'] = array(
    '#type' => 'textarea',
    '#title' => t('Small button'),
    '#default_value' => $button->small_button,
    '#required' => TRUE,
  );

  $form['enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enabled'),
    '#default_value' => $button->enabled,
  );

  $form['weight'] = array(
    '#type' => 'weight',
    '#title' => t('Weight'),
    '#delta' => 50,
    '#default_value' => $button->weight,
  );

  $form['submit'] = array('#type' => 'submit', '#value' => t('Save'));
  return $form;
}

/**
 * Submit handler for hook_addbutton().
 */
function sharebar_addbutton_submit($form, &$form_state) {
  if ($form_state['clicked_button']['#value'] == t('Delete')) {
    // Rebuild the form to confirm vocabulary deletion.
    $form_state['rebuild'] = TRUE;
    $form_state['confirm_delete'] = TRUE;
    return;
  }

  $buttons = unserialize(variable_get('sharebar_buttons', sharebar_buttons_def()));
  $form_state['values']['machine_name'] = preg_replace('/[^a-z0-9_]+/', '_', $form_state['values']['name']);
  if ($form_state['values']['old_machine_name'] != '' && $form_state['values']['old_machine_name'] != $form_state['values']['machine_name']) {
    unset($buttons[$form_state['values']['old_machine_name']]);
  }

  $buttons[$form_state['values']['machine_name']] = new stdClass();
  $buttons[$form_state['values']['machine_name']]->machine_name = $form_state['values']['machine_name'];
  $buttons[$form_state['values']['machine_name']]->name = $form_state['values']['name'];
  $buttons[$form_state['values']['machine_name']]->big_button = $form_state['values']['big_button'];
  $buttons[$form_state['values']['machine_name']]->small_button = $form_state['values']['small_button'];
  $buttons[$form_state['values']['machine_name']]->enabled = $form_state['values']['enabled'];
  $buttons[$form_state['values']['machine_name']]->weight = $form_state['values']['weight'];

  variable_set('sharebar_buttons', serialize($buttons));

  $form_state['redirect'] = 'admin/settings/sharebar';
}

/**
 * Form builder: delete buttons.
 */
function sharebar_button_confirm_delete(&$form_state) {
  $mname = arg(4);
  if ($mname) {
    $buttons = unserialize(variable_get('sharebar_buttons', sharebar_buttons_def()));
    if (is_array($buttons) && count($buttons)) {
      $button = $buttons[$mname];
    }
  }

  // Always provide entity id in the same form key as in the entity edit form.
  $form['machine_name'] = array('#type' => 'value', '#value' => $button->machine_name);
  $form['type'] = array('#type' => 'value', '#value' => 'button');
  $form['name'] = array('#type' => 'value', '#value' => $button->name);
  $form['delete'] = array('#type' => 'value', '#value' => TRUE);
  return confirm_form($form, t('Are you sure you want to delete the button %title?', array('%title' => $button->name)), 'admin/settings/sharebar', t('This action cannot be undone.'), t('Delete'), t('Cancel'));
}

/**
 * Submit handler for hook_button_confirm_delete().
 */
function sharebar_button_confirm_delete_submit($form, &$form_state) {
  $mname = arg(4);
  if ($form_state['values']['machine_name']) {
    $buttons = unserialize(variable_get('sharebar_buttons', sharebar_buttons_def()));
    if (is_array($buttons) && count($buttons)) {
      unset($buttons[$form_state['values']['machine_name']]);
      variable_set('sharebar_buttons', serialize($buttons));
    }
  }

  drupal_set_message(t('Deleted buttons %name.', array('%name' => $form_state['values']['name'])));
  watchdog('ShareBar', 'Deleted buttons %name.', array('%name' => $form_state['values']['name']), WATCHDOG_NOTICE);
  $form_state['redirect'] = 'admin/settings/sharebar';
  cache_clear_all();
  return;
}

/**
 * Callback to load existing machine name.
 */
function sharebar_machine_name_load($name) {
  $buttons = unserialize(variable_get('sharebar_buttons', sharebar_buttons_def()));
  if (is_array($buttons) && count($buttons)) {
    $button[] = $buttons[$name]->machine_name;
  }
  return reset($button);
}

/**
 * Form builder: Configure the sharebar system.
 */
function sharebar_settings() {
  drupal_add_css(drupal_get_path('module', 'sharebar') . '/css/colorpicker.css');
  drupal_add_js(drupal_get_path('module', 'sharebar') . '/js/colorpicker.js');
  drupal_add_js('jQuery(document).ready(function($) {
			var ids = ["edit-sharebar-bar-background","edit-sharebar-bar-border"];
			$.each(ids, function() {
				var id = this;
				$("#"+this).ColorPicker({
					onSubmit: function(hsb, hex, rgb, el) {
						$(el).val(hex);
						$(el).ColorPickerHide();
					},
					onBeforeShow: function () {
						$(this).ColorPickerSetColor(this.value);
					},
					onChange: function(hsb, hex, rgb, el) {
						$("#"+id).val(hex);
					}
				});
			});
		});
  ', 'inline');

  $form['buttonsset'] = array(
    '#type' => 'fieldset',
    '#title' => t('Buttons'),
    '#description' => t(''),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['buttonsset']['buttons'] = array(
    '#theme' => 'sharebar_buttons_table',
    '#weight' => 0,
  );

  // Add sharebar.
  $form['addsharebar'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add Sharebar'),
    '#description' => t('The following settings allow you to automatically add the Sharebar to your pages.'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $node_types = node_get_types();
  if (is_array($node_types) && count($node_types))
    foreach ($node_types as $key => $value) {
      $form['addsharebar']['sharebar_bar_posts_' . $value->type . '_enabled'] = array(
        '#type' => 'checkbox',
        '#title' => t('Automatically add Sharebar to content type ' . $value->name . '? (only affects content type ' . $value->name . ')'),
        '#default_value' => variable_get('sharebar_bar_posts_' . $value->type . '_enabled', TRUE),
      );
    }

  $form['addsharebar']['sharebar_bar_pages_enabled'] = array(
    '#type' => 'checkbox',
    '#title' => t('Automatically add Sharebar to Basic page? (only affects basic pages)'),
    '#default_value' => variable_get('sharebar_bar_pages_enabled', TRUE),
  );

  // Display options.
  $form['displayoptions'] = array(
    '#type' => 'fieldset',
    '#title' => t('Display options'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['displayoptions']['sharebar_bar_horizontal'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display horizontal Sharebar if the page is resized to less than 1000px?'),
    '#default_value' => variable_get('sharebar_bar_horizontal', TRUE),
  );

  $form['displayoptions']['sharebar_bar_oncontent'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display sharebar in content'),
    '#default_value' => variable_get('sharebar_bar_oncontent', TRUE),
    '#ahah' => array(
      'path' => 'sharebar/ahah/csscontainer/callback',
      //'path' => 'examples/ahah_example/autotextfields/callback',
      'wrapper' => 'csscontainer-wrapper',
      'effect' => 'fade',
    )
  );

  $form['displayoptions']['sharebar_bar_credit'] = array(
    '#type' => 'checkbox',
    '#title' => t('Display credit link back to the Sharebar plugin? If disabled, please consider donating.'),
    '#default_value' => variable_get('sharebar_bar_credit', TRUE),
  );

  $form['displayoptions']['csscontainer'] = array(
    '#title' => t("Custom Css Container"),
    '#prefix' => '<div id="csscontainer-wrapper">',
    '#suffix' => '</div>',
    '#type' => 'fieldset',

  );

  if (variable_get('sharebar_bar_oncontent', TRUE)) {
    $form['displayoptions']['csscontainer']['sharebar_bar_idcontent'] = array(
      '#type' => 'textfield',
      '#title' => t('Custom CSS Container when displayed in content region(only id selector is supported)'),
      '#size' => 10,
      '#default_value' => variable_get('sharebar_bar_idcontent', 'sharebarp'),
    );
  }
  else {
    $form['displayoptions']['csscontainer']['sharebar_bar_id'] = array(
      '#type' => 'textfield',
      '#title' => t('Custom CSS Container when not displayed in content region (only id selector is supported)'),
      '#size' => 10,
      '#default_value' => variable_get('sharebar_bar_id', 'sharebar'),
    );
  }

  $form['displayoptions']['sharebar_bar_toptoffset'] = array(
    '#type' => 'textfield',
    '#title' => t('Top offset'),
    '#size' => 10,
    '#default_value' => variable_get('sharebar_bar_toptoffset', 0),
  );

  $form['displayoptions']['sharebar_bar_position'] = array(
    '#type' => 'select',
    '#title' => t('Sharebar position'),
    '#default_value' => variable_get('sharebar_bar_position', 'left'),
    '#options' => array('left' => 'Left', 'right' => 'Right'),
  );

  $form['displayoptions']['sharebar_bar_leftoffset'] = array(
    '#type' => 'textfield',
    '#title' => t('Left Offset (used when positioned to left)'),
    '#size' => 10,
    '#default_value' => variable_get('sharebar_bar_leftoffset', 10),
  );

  $form['displayoptions']['sharebar_bar_rightoffset'] = array(
    '#type' => 'textfield',
    '#title' => t('Right Offset (used when positioned to right)'),
    '#size' => 10,
    '#default_value' => variable_get('sharebar_bar_rightoffset', 10),
  );

  $form['displayoptions']['sharebar_bar_width'] = array(
    '#type' => 'textfield',
    '#title' => t('Minimum width in pixels required to show vertical Sharebar to the left of post (cannot be blank)'),
    '#size' => 10,
    '#default_value' => variable_get('sharebar_bar_width', 1000),
  );

  // Customize.
  $form['customize'] = array(
    '#type' => 'fieldset',
    '#title' => t('Customize'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $form['customize']['sharebar_bar_swidth'] = array(
    '#type' => 'textfield',
    '#title' => t('Sharebar width'),
    '#size' => 10,
    '#default_value' => variable_get('sharebar_bar_swidth', 75),
  );

  $form['customize']['sharebar_bar_twitter_username'] = array(
    '#type' => 'textfield',
    '#title' => t('Twitter username'),
    '#size' => 10,
    '#default_value' => variable_get('sharebar_bar_twitter_username', 'themesnap'),
  );

  $form['customize']['sharebar_bar_background'] = array(
    '#type' => 'textfield',
    '#title' => t('Sharebar background color'),
    '#size' => 10,
    '#default_value' => variable_get('sharebar_bar_background', 'FFFFFF'),
  );

  $form['customize']['sharebar_bar_border'] = array(
    '#type' => 'textfield',
    '#title' => t('Sharebar border color'),
    '#size' => 10,
    '#default_value' => variable_get('sharebar_bar_border', 'CCCCCC'),
  );

  $form['#attributes'] = array('enctype' => 'multipart/form-data');
  return system_settings_form($form);
}

/**
 * Function to get settings.
 */
function sharebar_get_setting_str($str, $tag) {
  $n1 = strpos($str, $tag) + drupal_strlen($tag) + 1;
  $n2 = strpos($str, $tag, $n1) - 2;
  return drupal_substr($str, $n1, $n2 - $n1);
}

/**
 * Provide a single block from the administration menu as a page.
 */
function sharebar_admin_menu_block_page() {
  $item = menu_get_item();
  if ($content = system_admin_menu_block($item)) {
    $output = theme('admin_block_content', array('content' => $content));
  }
  else {
    $output = t('You do not have any administrative items.');
  }
  return $output;
}

/**
 * AHAH callback for custom css container id.
 */
function sharebar_ahah_csscontainer_callback() {
  $form = sharebar_ahah_csscontainer_callback_helper();

  $csscontainer =  $form['displayoptions']['csscontainer'];

  // Remove the prefix/suffix wrapper so we don't double it up.
  unset($csscontainer['#prefix'], $csscontainer['#suffix']);

  // Render the output.
  $output = theme('status_messages');
  $output .= drupal_render($csscontainer);

  // Final rendering callback.
  drupal_json(array('status' => TRUE, 'data' => $output));
  exit();
}


