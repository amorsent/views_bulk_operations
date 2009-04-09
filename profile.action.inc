<?php
// $Id$

function views_bulk_operations_profile_action_info() {
  if (!module_exists('profile')) return array();
  return array('views_bulk_operations_profile_action' => array(
    'type' => 'user',
    'description' => t('Modify profile fields'),
    'configurable' => TRUE,
  ));
}

function views_bulk_operations_profile_action_form($context) {
  $form = array();
  foreach (profile_categories() as $category) {
    $fields = _profile_get_fields($category['name']);
    while ($field = db_fetch_object($fields)) {
      $form[$field->name .'_check'] = array(
        '#type' => 'checkbox',
        '#title' => t('Edit field %title', array('%title' => $field->title)),
      );
    }
    $form += profile_form_profile($context, NULL, $category['name']);
  }
  return $form;
}

function views_bulk_operations_profile_action_validate($form, $form_state) {
  foreach (profile_categories() as $category) {
    profile_validate_profile($form_state['values'], $category['name']);
  }
}

function views_bulk_operations_profile_action_submit($form, $form_state) {
  $return = array();
  foreach (profile_categories() as $category) {
    $fields = _profile_get_fields($category['name']);
    while ($field = db_fetch_object($fields)) {
      $return[$field->name .'_check'] = $form_state['values'][$field->name .'_check'];
      $return[$field->name] = $form_state['values'][$field->name];
    }
  }
  return $return;
}

function views_bulk_operations_profile_action(&$user, $context) {
  foreach (profile_categories() as $category) {
    _views_bulk_operations_profile_action_save($context, $user, $category['name']);
  }
}

function _views_bulk_operations_profile_action_save(&$edit, &$user, $category, $register = FALSE) {
  $result = _profile_get_fields($category, $register);
  while ($field = db_fetch_object($result)) {
    if (!$edit[$field->name .'_check']) continue; // skip if the user didn't check that field

    if (_profile_field_serialize($field->type)) {
      $edit[$field->name] = serialize($edit[$field->name]);
    }
    db_query("DELETE FROM {profile_values} WHERE fid = %d AND uid = %d", $field->fid, $user->uid);
    db_query("INSERT INTO {profile_values} (fid, uid, value) VALUES (%d, %d, '%s')", $field->fid, $user->uid, $edit[$field->name]);
    // Mark field as handled (prevents saving to user->data).
    $edit[$field->name] = NULL;
  }
}
