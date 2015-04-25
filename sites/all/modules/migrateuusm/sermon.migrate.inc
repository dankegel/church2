<?php

// This imports sermons, all fields, including audio.  Audio is handled
// along with all the other fields conained in the Sermon migration. The
// special thing about files is that you have to populate the file_managed
// table as well as the field table (field_data_field_audio in this case).
// More comments below on how this is done.

class SermonMigration extends Migration
{
    public
    function __construct($arguments)
    {
        parent::__construct($arguments);

        // Avoid known line ending issue: "Invalid data value" at drupal.org/node/1152158#InvalidDataValue
        ini_set('auto_detect_line_endings', TRUE);

        $this->dependencies = array('Users');

        $this->description = t("import sermons");

        // Make a query that gets everything. Do a lot of joins.  This query restricts
        // to records known to have attached audio files.

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
        ))->fields('s', array(
            'nid',
            'field_minister_value',
            'field_pulpit_host_value',
            'field_sermon_date_value',
            'field_sermon_theme_value'
        ))->fields('f', array(
            'filename',
            'filepath',
            'fid'
        ))->fields('a', array(
            'field_audio_fid'
        ))->fields('st', array(
            'field_sermon_text_value',
            'nid'
        ));
        $query->join('node_revisions', 'nr', 'n.nid=nr.nid');
        $query->join('content_type_sermon', 's', 'n.nid=s.nid');
        $query->join('content_field_audio', 'a', 's.nid=a.nid');
        $query->join('files', 'f', 'a.field_audio_fid=f.fid');
        $query->join('content_field_sermon_text', 'st', 'st.nid=n.nid');
        $query->condition('n.type', 'sermon', '=');
        $query->condition('st.field_sermon_text_value', '', '!=');

        $options = array(
            'track_changes' => 1,
            'map_joinable' => FALSE,
        );
        $this->source = new MigrateSourceSQL($query, array() , NULL, $options);
        $this->destination = new MigrateDestinationNode('sermons');

        // This part creates the lookup table that keeps track of the old nid and the new nid.  By keeping track of
        // this, it is possible to avoid re-importing the same record more than once, and it allows you to
        // roll back the entire migration. First it maps, and then it calls on getKeySchema which consults
        // Drupal structure for the Sermon content type.  I think.

        $this->map = new MigrateSQLMap($this->machineName, array(
            'nid' => array(
                'type' => 'varchar',
                'length' => 6,
                'not null' => TRUE,
                'description' => 'Account ID.',
                'alias' => 'n'
            )
        ) , MigrateDestinationNode::getKeySchema());

        // Most of these mappings are self evident.  Except for field_audio

        $this->addFieldMapping('nid', 'n.nid');
        $this->addFieldMapping('is_new')->defaultValue(TRUE)->description(t('Build the new sermon (0|1)'));
        $this->addFieldMapping('title', 'title');
        $this->addFieldMapping('status', 'status');
        $this->addFieldMapping('created', 'created');
        $this->addFieldMapping('changed', 'changed');
        $this->addFieldMapping('promote', 'promote');
        $this->addFieldMapping('sticky', 'sticky');
        $this->addFieldMapping('body', 'field_description');
        $this->addFieldMapping('body:format')->defaultValue('full_html');

        // $this->addFieldMapping('body:summary','teaser');

        $this->addFieldMapping('field_minister_speaker', 'field_minister_value');
        $this->addFieldMapping('field_pulpit_host', 'field_pulpit_host_value');
        $this->addFieldMapping('field_sermon_date', 'field_sermon_date_value');
        $this->addFieldMapping('field_sermon_text', 'field_sermon_text_value');
        $this->addFieldMapping('field_sermon_text:format')->defaultValue('full_html');
        $this->addFieldMapping('field_sermon_theme', 'field_sermon_theme_value');

        // field_audio maps to an asset depending on the file_class assigned to handle all the logic and data
        // that has to fill up the files_managed table and the field_audio.  When you use the class MigrateFileUri
        // that means the value of what's mapped to field_audio is a url, relative to the field_audio
        // source_directory.  So since the source directory path is the entire url including http - because
        // it's remote -- all you need to map to field_audio is the filename.

        $this->addFieldMapping('field_audio', 'filename');
        $this->addFieldMapping('field_audio:file_class')->defaultValue('MigrateFileUri');
        $this->addFieldMapping('field_audio:source_dir')->defaultValue('http://www.peggyo.net/fuller/sites/default/files/audio/');

        // This is supposed to make it so you don't delete the files if you roll back but it doesn't work.
        // Need to find out why this does not work.  If you roll back, all 500 mb of audio files you patiently
        // imported will be erased.

        $this->addFieldMapping('field_audio:preserve_files', TRUE);
    }
}
