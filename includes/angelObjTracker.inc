<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 1/5/2019
 * Time: 10:56 PM
 */
 
class angelTracker extends angeldb {

    public $personID         = 0;        // person ID of the person being tracked
    public $companyID        = 0;        // company ID if the person is a founder w/a company
    public $investorID       = 0;        // investor doing the tracking
    public $trackerDate      = "";       // date tracker entered
    public $lastStatusUpdate = "";       // date of the most recent status update
    public $nextContactDate  = "";       // next date where this tracker goes live
    public $status           = "";       // current status "tracked", "emailed", "scheduled", "met"
    public $type             = "";       // tracker type: YCfounder, founder, investor, other, friend -- UNUSED
    public $frequency        = "";       // how often to track: monthly, quarterly, yearly
    public $start            = "";       // when to begin tracking: Now, Next Month, Next Quarter
    public $notes            = "";       // notes about why tracking / meeting

    function __construct() {
        parent::__construct();

        $this->column_names = "personID,companyID,investorID,trackerDate,lastStatusUpdate,nextContactDate,status,type,frequency,start,notes";
        $this->table_name = "trackertable";
    }


}  // angelTracker