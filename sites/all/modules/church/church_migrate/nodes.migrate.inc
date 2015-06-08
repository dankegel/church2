<?php
# Generic node migration base class
# Extended by ChurchRoomMigration, etc.
# See https://www.drupal.org/node/1513766

abstract class NodeMigration extends Migration {
  public function __construct(array $arguments) {
    parent::__construct($arguments);
    $type = isset($arguments['type']) ? $arguments['type'] : NULL;
    $new_type = isset($arguments['new type']) ? $arguments['new type'] : $type;
    $options = array('track_changes' => 1);
    $this->description = t('Migrate nodes');
    $this->map = new MigrateSQLMap($this->machineName,
      array(
        'nid' => array(
          'type' => 'int',
          'unsigned' => TRUE,
          'not null' => TRUE,
          'description' => 'd6 Unique Node ID',
          'alias' => 'n',
        )
      ),
      MigrateDestinationNode::getKeySchema()
    );

    $query = $this->node_query($type);
    $this->set_highwater_field();
    $this->set_source($query);
    $this->destination = new MigrateDestinationNode($new_type);
    $this->node_field_mapping();
  }

  public function set_highwater_field() {
    $this->highwaterField = array(
      'name' => 'changed',
      'alias' => 'n',
    );
  }

  public function node_query($type) {
    $query = Database::getConnection('default','for_migration')
      ->select('node', 'n')
      ->fields('n', array('nid', 'vid', 'title', 'uid', 'status', 'created', 'changed', 'comment', 'promote', 'moderate', 'sticky'))
      ->condition('n.type', $type);
    $query->join('node_revisions', 'nr', 'nr.vid = n.vid');
    $query->fields('nr', array('body', 'teaser', 'format'));
    return $query;
  }

  public function set_source($query) {
    $this->source = new MigrateSourceSQL($query);
    $this->source->setMapJoinable(FALSE);
  }

  public function node_field_mapping() {
    // Make the mappings
    $this->addSimpleMappings(
      array(
        'title',
        'status',
        'created',
        'changed',
        'comment',
        'promote',
        'moderate',
        'sticky',
      )
    );
    $this->addFieldMapping('uid', 'uid')->sourceMigration('Users');
    $this->addFieldMapping('body', 'body');
    $this->addFieldMapping('body:format', 'format');
    $this->addFieldMapping('language')->defaultValue('en');
    $this->addFieldMapping('is_new')->defaultValue(TRUE);
  }

  public function prepareRow($current_row) {
    $formats = array(
      '1' => 'filtered_html',
      '2' => 'full_html',
      '3' => 'plain_text',
      '4' => 'markdown',
    );
    $current_row->format = isset($formats[$current_row->format]) ? $formats[$current_row->format] : 'plain_text';
  }
}
?>
