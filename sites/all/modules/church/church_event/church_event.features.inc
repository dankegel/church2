<?php
/**
 * @file
 * church_event.features.inc
 */

/**
 * Implements hook_node_info().
 */
function church_event_node_info() {
  $items = array(
    'event' => array(
      'name' => t('Event'),
      'base' => 'node_content',
      'description' => t('A scheduled event which will appear on the site calendar.  May include attachments.'),
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
  );
  drupal_alter('node_info', $items);
  return $items;
}
