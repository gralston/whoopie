<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 7/8/14
 * Time: 10:56 PM
 */
 
class angelNote extends angeldb {

    public $companyID       = 0;        // if there is a relevant company
    public $personID        = 0;        // if a note on a particular person
    public $investorID      = 0;        // investor making the note
    public $public          = 0;        // a boolean which is true if everyone can see this note
    public $type            = "";       // note type: update, meeting, note, valuation update, exit prediction, stock split
    public $noteDate        = "";       // date of note
    public $notes           = "";       // whatever you like
    /*
     * valuation update / company
     */
    public $round           = "";       // most recent round raised
    public $invested        = 0;        // amount invested in most recent round
    public $valuation       = 0;        // latest company valuation (post money)
    public $FDshares        = 0;        // fully diluted shares outstanding
    public $sharePrice      = 0;        // latest price per share or predicted share price if a prediction type
    /*
     * stock split
     */
    public $splitRatio      = 0.0;      // ratio of new shares to old shares in split (2.0 = 2/1  split)

    function __construct() {
        parent::__construct();

        $this->column_names = "companyID,personID,investorID,public,type,noteDate,notes,round,invested,valuation,FDshares,sharePrice,splitRatio";
        $this->table_name = "notestable";
    }


}  // angelNote