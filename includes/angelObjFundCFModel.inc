<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 7/8/14
 * Time: 10:56 PM
 *
 * A model consists of your convertible and then multiple other converts plus a qualified financing
 */
 /*
  * Model for future cash flows - distributions only. Payments are assumed to be an even amount over four years
  * from the first commitment
  */
class angelFundCFModel extends angeldb {

    protected $refcount     = 0;        // increment this every time the model is  loaded

    public $investorID      = 0;        // owner of this model
    public $fundID          = 0;        // this is a model for this fund
    public $creationDate    = "";       // date model created
    public $name            = "";       // name of this model (may not be needed)
    public $years           = 10;       // number of distribution years in this model
    public $percentPerYear  = "";       // query string format: 1:%&2:%&... (1 is current year, 2 is next year,...)
    public $sharing = 0;                // if null not shared, otherwise the share hash


    function __construct() {
        parent::__construct();

        $this->column_names = "investorID,fundID,creationDate,name,years,percentPerYear";
        $this->table_name = "fundmodeltable";
    }


    function incrementRefcount() {
        $id = $this->id;
        $conn = $this->conn;
        $table = $this->table_name;

        $query = "UPDATE $table SET refcount=refcount+1 WHERE id = $id";
        $result = mysqli_query($conn,$query);
        if ($result == NULL)
            angeldie("model refcount query failed");

    }

    /*
     * query string input, returns results in array[0] : percent for each year.
     */
    function decodeYears($percentPerYear,&$result) {
        parse_str($percentPerYear, $result);
    }

 /*
  * array input, return query string percent per year (yr0=percent&yr1=percent& ... )
  */
    function encodeYears($years) {
        $qs = "";

        for ($i = 0; $i < sizeof($years); $i++) {
            $qs .= $i+1 . "=" . $years[$i] . "&";
        }
        angeldie("encodeYears: qs=" . $qs);      // debug
        return($qs);

    }



}  // angelfundcfmodel