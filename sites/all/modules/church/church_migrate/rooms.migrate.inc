<?php
# See https://www.drupal.org/node/1513766
# and http://btmash.com/article/2011-03-25/migrating-content-part-2-nodes

class ChurchRoomsMigration extends NodeMigration {
  public function __construct($arguments)
  {
    parent::__construct($arguments + array('type' => 'room'));

    // Tell it that 'drush mi Users' has to come first
    $this->dependencies = array('Users');
    $this->description = t("import rooms");
  }

  # Override parent's nodeQuery; add fields specific to Rooms
  public function nodeQuery()
  {
    $query = parent::nodeQuery();
    $query->addField('n', 'field_image');
    return $query;
  }
}
?>
