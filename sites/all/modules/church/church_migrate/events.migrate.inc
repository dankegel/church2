<?php

# Resistance is futile, you will use migrate_d2d

class ChurchEventsMigration extends ChurchNodeMigration {
  public function __construct($arguments)
  {
    parent::__construct($arguments);
    $this->addSimpleMappings(array(
      'field_datetime', 
      'field_event_contact',
      'field_event_contact_email',
      'field_event_contact_phone',
      'field_event_setup_details',
      'field_file_attachments',
      'field_rental_description',
      'field_room',
    ));
    $this->addFieldMapping('field_datetime:to', 'field_datetime:value2');
    $this->addFieldMapping('field_datetime:timezone', 'field_datetime:timezone') ->defaultvalue('America/Los_Angeles'));
    $this->addFieldMapping('field_datetime:rrule', 'field_datetime:rrule');
  }
}
?>
