<?php
# See http://btmash.com/article/2011-03-25/migrating-content-part-2-nodes

function convert_input_format_from_d6_to_d7($format) {
  switch ($format) {
    case 1:
      return 'filtered_html';
    case 2:
      return 'plain_text';
    case 3:
      return 'full_html';
    case 4:
      return 'full_html_with_markdown';
    default:
      return 'plain_text';
  }
}

class ChurchRoomsMigration extends Migration {
  public function __construct($arguments)
  {
    parent::__construct($arguments);

    // Tell it that 'drush mi Users' has to come first
    $this->dependencies = array('Users');

    $this->description = t("import rooms");
    $options = array('track_changes' => 1);

    $query = Database::getConnection('default','for_migration')->select('rooms', 'u');
    $query->fields('u', array('nid', 'title', 'path', 'body', 'field_image'))
      ->orderBy('u.nid', 'ASC');

    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'nid' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'D6 Unique Node ID',
          'alias' => 'u',
        )
      ),
      MigrateDestinationNode::getKeySchema()
    );
    /* avoid joins, we don't have permission to write to source */
    $this->source = new MigrateSourceSQL($query, array(), NULL, array('map_joinable' => FALSE));
    $this->destination = new MigrateDestinationNode('room');

    $this->addFieldMapping('title', 'title')->dedupe('rooms', 'title');
    $this->addFieldMapping('path', 'path');
    $this->addFieldMapping('promote', 'promote');
    $this->addFieldMapping('sticky', 'sticky');
    $this->addFieldMapping('comment', 'comment');
    $body_arguments = MigrateTextFieldHandler::arguments(NULL, filter_default_format(), NULL);
    $this->addFieldMapping('body', 'body')->arguments($body_arguments);
    $this->addFieldMapping('field_image')->defaultValue(0);

    // Unmapped source fields
    $this->addUnmigratedSources(array('vid', 'type', 'language', 'moderate', 'tnid', 'translate'));
  }
}
?>
