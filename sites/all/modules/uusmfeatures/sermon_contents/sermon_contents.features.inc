<?php
/**
 * @file
 * sermon_contents.features.inc
 */

/**
 * Implements hook_node_info().
 */
function sermon_contents_node_info() {
  $items = array(
    'sermons' => array(
      'name' => t('Sermons'),
      'base' => 'node_content',
      'description' => t('Sermons and associated assets.'),
      'has_title' => '1',
      'title_label' => t('Title'),
      'help' => '',
    ),
  );
  drupal_alter('node_info', $items);
  return $items;
}
