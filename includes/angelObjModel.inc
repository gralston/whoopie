<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 7/8/14
 * Time: 10:56 PM
 *
 * A model consists of your convertible and then multiple other converts plus a qualified financing
 */
 
class angelModel extends angeldb {

    protected $refcount     = 0;        // increment this every time the model is  loaded

    public $investor_id     = 0;        // owner of this model
    public $creationDate    = "";       // date model created
    public $name            = "";       // name of this model
    public $numberConverts = 0;          // total number of converts in this round
    public $numberFounders = 0;          // total number of founders in the company
    public $founders = "";              // query string w/founders as 'name1'=shares&'name2'=shares
    public $numberOtherCommon = 0;      // total number of other common stock holders in the company
    public $otherCommon = "";              // query string w/common holders as 'name1'=shares&'name2'=shares

    // qualified financing //
    public $vcInvestment = 0;            // how much was invested in the qualified round (total)
    public $yourInvestment = 0;          // angel investment in the equity round (if any)
    public $preValuation = 0;            // premoney valuation
    public $postOptionsPercent = 0;      // post money option pool
    public $fdPreMoneyShares = 0;       // fully diluted pre-money shares issued / outstanding (includes all options)
    public $preOptionPoolSize = 0;      // total option pool pre money
    public $preGrantedOptions = 0;      // options which have been granted pre money
    public $prePromisedOptions = 0;     // options promised pre money which are consider part of fd shares
    public $preUnissuedOptions = 0;     // pre-money options left in pool which have not yet been allocated.
    public $sharing = 0;                // if null not shared, otherwise the share hash


    function __construct() {
        parent::__construct();

        $this->column_names = "investor_id,creationDate,name,numberConverts,numberFounders,founders,numberOtherCommon,otherCommon,vcInvestment,yourInvestment,preValuation,preOptionPoolSize,preGrantedOptions,prePromisedOptions,preUnissuedOptions,postOptionsPercent,fdPreMoneyShares,sharing";
        $this->table_name = "modeltable";
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


}  // investment