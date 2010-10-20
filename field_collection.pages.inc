<?php
// $Id$

/**
 * @file
 * Provides the field_collection view / edit / delete pages.
 */

// TODO: fix being embedded in a host with revisions.

/**
 * field_collection view page.
 */
function field_collection_page_view($field_collection) {
  // @todo: Set breadcrumb including the host.
  drupal_set_title($field_collection->label());
  return $field_collection->view();
}

/**
 * Form for editing an field_collection.
 * @todo implement hook_forms().
 */
function field_collection_form($form, &$form_state, $field_collection) {
  if (!isset($field_collection->is_new)) {
    drupal_set_title($field_collection->label());
  }
  $form_state += array('field_collection' => $field_collection);

  // Hack: entity_form_field_validate() needs the bundle to be set.
  // @todo: Fix core and remove the hack.
  $form['field_name'] = array('#type' => 'value', '#value' => $field_collection->field_name);

  field_attach_form('field_collection', $field_collection, $form, $form_state);

  $form['actions'] = array('#type' => 'actions', '#weight' => 50);
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save'),
    '#weight' => 5,
  );
  return $form;
}

/**
 * Validation callback.
 */
function field_collection_form_validate($form, &$form_state) {
  entity_form_field_validate('field_collection', $form, $form_state);
}

/**
 * Submit builder. Extracts the form values and updates the entity.
 */
function field_collection_form_submit_build_field_collection($form, $form_state) {
  entity_form_submit_build_entity('field_collection', $form_state['field_collection'], $form, $form_state);
  return $form_state['field_collection'];
}

/**
 * Submit callback that permanently saves the changes to the entity.
 */
function field_collection_form_submit($form, &$form_state) {
  $field_collection = field_collection_form_submit_build_field_collection($form, $form_state);
  $field_collection->save();
  drupal_set_message(t('The changes have been saved.'));
  $form_state['redirect'] = $field_collection->path();
}

/**
 * Form for deleting an field_collection.
 */
function field_collection_delete_confirm($form, &$form_state, $field_collection) {
  $form_state += array('field_collection' => $field_collection);
  return confirm_form($form,
    t('Are you sure you want to delete %label?', array('%label' => $field_collection->label())),
    $field_collection->path(),
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Submit callback for deleting an field_collection.
 */
function field_collection_delete_confirm_submit($form, &$form_state) {
  $field_collection = $form_state['field_collection'];
  $field_collection->delete();
  drupal_set_message(t('%label has been deleted.', array('%label' => drupal_ucfirst($field_collection->label()))));
  $form_state['redirect'] = '<front>';
}

/**
 * Add a new field_collection.
 */
function field_collection_add($field_name, $entity_type, $entity_id) {
  $info = entity_get_info();
  if (!isset($info[$entity_type])) {
    return MENU_NOT_FOUND;
  }
  $result = entity_load($entity_type, array($entity_id));
  $entity = reset($result);
  if (!$entity) {
    return MENU_NOT_FOUND;
  }
  // Ensure the given entity is of a bundle that has an instance of the field.
  list($id, $rev_id, $bundle) = entity_extract_ids($entity_type, $entity);
  $instance = field_info_instance($entity_type, $field_name, $bundle);
  if (!$instance) {
    return MENU_NOT_FOUND;
  }

  // Check field cardinality.
  $field = field_info_field($field_name);
  // @todo: Support language specific fields?
  if (!($field['cardinality'] == FIELD_CARDINALITY_UNLIMITED || !isset($entity->{$field_name}[LANGUAGE_NONE]) || count($entity->{$field_name}[LANGUAGE_NONE]) < $field['cardinality'])) {
    drupal_set_message(t('Too many items.'), 'error');
    return '';
  }

  drupal_set_title(t('Add new @instance_label', array('@instance_label' => $instance['label'])));
  $field_collection = entity_create('field_collection', array('field_name' => $field_name));
  // Add the host entity to the field_collection such that it is picked upon save
  // and the link between the entities is created. see field_collection::save().
  $field_collection->host_entity = $entity;
  $field_collection->host_entity_type = $entity_type;
  return drupal_get_form('field_collection_form', $field_collection);
}
