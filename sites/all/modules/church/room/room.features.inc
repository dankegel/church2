<?php
/**
 * @file
 * room.features.inc
 */

/**
 * Implements hook_node_info().
 */
function room_node_info() {
  $items = array(
    'room' => array(
      'name' => t('Room'),
      'base' => 'node_content',
      'description' => t('Content type for handling room bookings'),
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
  );
  drupal_alter('node_info', $items);
  return $items;
}
