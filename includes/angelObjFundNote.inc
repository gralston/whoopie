<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 7/8/14
 * Time: 10:56 PM
 */
 
class angelFundNote extends angelNote {

    public $fundID          = 0;        // if there is a relevant fund
    public $personID        = 0;        // if a note on a particular person
    public $investorID      = 0;        // investor making the note
    public $public          = 0;        // a boolean which is true if everyone can see this note
    public $type            = "";       // note type: update, meeting, note, valuation update, exit prediction
    public $noteDate        = "";       // date of note
    public $notes           = "";       // whatever you like
    /*
     * valuation update / company
     */

    /*
     * valuation update / fund
     */
    public $capitalAccount = 0;         // most recent capital account value
    public $predictedReturn = 0;        // prediction for ultimate total future return (not incl actuals)
                                        // in other words, is is what the capital account WILL BE

    function __construct() {
        parent::__construct();

        $this->column_names = "fundID,personID,investorID,public,type,noteDate,notes,capitalAccount,predictedReturn";
        $this->table_name = "fundnotestable";
    }


}  // angelFundNote