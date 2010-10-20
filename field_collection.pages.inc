<?php
// $Id$

/**
 * @file
 * Provides the embeddable view / edit / delete pages.
 */

// TODO: fix being embedded in a host with revisions.

/**
 * Embeddable view page.
 */
function embeddable_page_view($embeddable) {
  // @todo: Set breadcrumb including the host.
  drupal_set_title($embeddable->label());
  return $embeddable->view();
}

/**
 * Form for editing an embeddable.
 * @todo implement hook_forms().
 */
function embeddable_form($form, &$form_state, $embeddable) {
  if (!isset($embeddable->is_new)) {
    drupal_set_title($embeddable->label());
  }
  $form_state += array('embeddable' => $embeddable);

  // Hack: entity_form_field_validate() needs the bundle to be set.
  // @todo: Fix core and remove the hack.
  $form['field_name'] = array('#type' => 'value', '#value' => $embeddable->field_name);

  field_attach_form('embeddable', $embeddable, $form, $form_state);

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
function embeddable_form_validate($form, &$form_state) {
  entity_form_field_validate('embeddable', $form, $form_state);
}

/**
 * Submit builder. Extracts the form values and updates the entity.
 */
function embeddable_form_submit_build_embeddable($form, $form_state) {
  entity_form_submit_build_entity('embeddable', $form_state['embeddable'], $form, $form_state);
  return $form_state['embeddable'];
}

/**
 * Submit callback that permanently saves the changes to the entity.
 */
function embeddable_form_submit($form, &$form_state) {
  $embeddable = embeddable_form_submit_build_embeddable($form, $form_state);
  $embeddable->save();
  drupal_set_message(t('The changes have been saved.'));
  $form_state['redirect'] = $embeddable->path();
}

/**
 * Form for deleting an embeddable.
 */
function embeddable_delete_confirm($form, &$form_state, $embeddable) {
  $form_state += array('embeddable' => $embeddable);
  return confirm_form($form,
    t('Are you sure you want to delete %label?', array('%label' => $embeddable->label())),
    $embeddable->path(),
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Submit callback for deleting an embeddable.
 */
function embeddable_delete_confirm_submit($form, &$form_state) {
  $embeddable = $form_state['embeddable'];
  $embeddable->delete();
  drupal_set_message(t('%label has been deleted.', array('%label' => drupal_ucfirst($embeddable->label()))));
  $form_state['redirect'] = '<front>';
}

/**
 * Add a new embeddable.
 */
function embeddable_add($field_name, $entity_type, $entity_id) {
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
  $embeddable = entity_create('embeddable', array('field_name' => $field_name));
  // Add the host entity to the embeddable such that it is picked upon save
  // and the link between the entities is created. see Embeddable::save().
  $embeddable->host_entity = $entity;
  $embeddable->host_entity_type = $entity_type;
  return drupal_get_form('embeddable_form', $embeddable);
}
