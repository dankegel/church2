<?php

# Resistance is futile, you will use migrate_d2d

class ChurchRoomsMigration extends ChurchNodeMigration {
  public function __construct($arguments)
  {
    parent::__construct($arguments);
    # No custom fields to deal with, yay
  }
}
?>
