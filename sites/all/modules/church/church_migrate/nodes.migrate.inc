<?php
# Generic node migration base class
# Extended by ChurchRoomMigration, etc.
# See https://dev.acquia.com/blog/drupal-drupal-data-migration-part-1-basics and
# migrate_d2d_example

abstract class ChurchNodeMigration extends DrupalNode6Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);

    $this->addFieldMapping('field_legacy_nid', 'nid')
         ->description('We have a common field to save the D6 nid');
  }
}
?>
