<?php
// The following class imports all the events and dates.  Everything gets
// put into the myevents content type.  The fancy trick here is to fetch
// a comma delimited list in the query, using just mysql. GROUP_CONCAT
// allows you make comma delimited lists out of dates you get from
// content_field_datetime which allows for multiple dates associated with
// the same node id.  Then you group by node id, and send the comma delimited
// list to the date field in D7.  The ability to translate a comma delimited
// list into records is built in.
class CalendarMigration extends Migration
{
    public function __construct($arguments)
    {
        parent::__construct($arguments);

        // Avoid known line ending issue: "Invalid data value" at drupal.org/node/1152158#InvalidDataValue
        ini_set('auto_detect_line_endings', TRUE);

        $this->dependencies = array('Users');

        $this->description = t("import data to put into Event fields");
        $query = Database::getConnection('default', 'for_migration')->select('node', 'n')->fields('n', array(
            'nid',
            'uid',
            'status',
            'created',
            'changed',
            'promote',
            'sticky'
        ))->fields('nr', array(
            'title',
            'body',
            'teaser'
        ))->fields('c', array(
            'nid',
            'field_event_contact_value'
        ))->fields('e', array(
            'nid',
            'field_event_contact_email_email'
        ))->fields('p', array(
            'nid',
            'field_event_contact_phone_value'
        ))->fields('m', array(
            'nid',
            'field_datetime_value',
            'field_datetime_value2',
            'field_datetime_rrule'
        ));
        $query->join('node_revisions', 'nr', "n.nid=nr.nid");
        $query->join('content_field_event_contact', 'c', "n.nid=c.nid");
        $query->join('content_field_event_contact_email', 'e', "n.nid=e.nid");
        $query->join('content_field_event_contact_phone', 'p', "n.nid=p.nid");
        $query->join('content_field_datetime', 'm', "n.nid = m.nid");
        $query->condition('n.type', 'event', '=');
        $query->addExpression("GROUP_CONCAT(m.field_datetime_value SEPARATOR '||')", 'multipledatetime');
        $query->addExpression("GROUP_CONCAT(m.field_datetime_value2 SEPARATOR '||')", 'multipledatetime2');
        $query->addExpression("GROUP_CONCAT(m.field_datetime_rrule SEPARATOR '||')", 'multipledatetimerrule');
        $query->groupBy('m.nid');

        $options = array(
            'track_changes' => 1,
            'map_joinable' => FALSE
        );
        $this->source = new MigrateSourceSQL($query, array() , NULL, $options);
        $this->destination = new MigrateDestinationNode('myevents');
        $this->map = new MigrateSQLMap($this->machineName, array(
            'nid' => array(
                'type' => 'varchar',
                'length' => 6,
                'not null' => TRUE,
                'description' => 'Account ID.',
                'alias' => 'n'
            )
        ) , MigrateDestinationNode::getKeySchema());
        $this->addFieldMapping('nid', 'n.nid'); // Connecting the D6 nid to the D7 nid
        $this->addFieldMapping('is_new')->defaultValue(TRUE)->description(t('Build the new user (0|1)'));

        // Mapped fields

        $this->addFieldMapping('title', 'title');
        $this->addFieldMapping('uid', 'uid');
        $this->addFieldMapping('status', 'status');
        $this->addFieldMapping('created', 'created');
        $this->addFieldMapping('changed', 'changed');
        $this->addFieldMapping('promote', 'promote');
        $this->addFieldMapping('sticky', 'sticky');
        $this->addFieldMapping('body', 'body');
        $this->addFieldMapping('body:format')->defaultValue('full_html');
        $this->addFieldMapping('body:summary', 'teaser');
        $this->addFieldMapping('field_contact', 'field_event_contact_value');
        $this->addFieldMapping('field_contact_email', 'field_event_contact_email_email');
        $this->addFieldMapping('field_contact_phone', 'field_event_contact_phone_value');
        $this->addFieldMapping('field_event_date', 'multipledatetime')->separator('||');
        $this->addFieldMapping('field_event_date:value2', 'multipledatetime2')->separator('||');
        $this->addFieldMapping('field_event_date:rrule', 'multipledatetimerrule')->separator('||');
    }

    public function prepareRow($current_row)
    {
        // ??
        date(DATE_ATOM, strtotime($current_row->multipledatetime));
        date(DATE_ATOM, strtotime($current_row->multipledatetime2));
    }
}

