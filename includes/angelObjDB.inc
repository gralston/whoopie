<?php



/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 6/2/14
 * Time: 12:44 PM
 */


/*
 * angeldb - the key base class for the entire ik12 database system. Here  the properties are documented below. The
 * supported methods include:
 *      connect()  -- connect to the specified db and store the result in $GLOBALS["conn"] and this->conn
 *      getRows()
 *      error()
 *      createMap()
 *
 *
 */
class angeldb {
    protected $conn;            // if set a db connection id
    protected $db;              // i think this is deprecated.  check later.
    protected $hostname;        // machine on which the db lives - usually localhost:3306
    protected $dbname;          // the name of the db to which we are connecting, which should always be "ik12db"
    protected $dbuser;          // root user
    protected $dbpass;          // password
    protected $mysqli_err;      // stored error from db operation
    protected $table_name;      // table name
    protected $column_names;    // all column names for this table

    public    $id = 0;          // standard db fields id = key and last_update (auto filled date of last update)
    public    $last_update = "";


    function __construct() {

        $this->conn         = null;
        $this->hostname     = "localhost:3306";
        $this->dbname       = "whoopiedb";
        $this->dbuser       = "root";
        $this->dbpass       = "ralston";


    }  // construct()

    /*
     * Since conn is a protected property, if there is a separate object which has a connection, this allows
     * that connection to be used by other objects.
     */
    function shareconn($conn) {
        $this->conn = $conn;
    }
    function returnconn() {
        return($this->conn);
    }
    /*
     * useful if same object used in multiple tables
     */
    function changeTable($tab) {
        $this->table_name = $tab;
    }
    /*
     * Create database connection and select the db
     */
    function connect() {

        if ($this->conn)            // if already connected you are done.
            return($this->conn);

        // echo "username/password: $this->dbuser / $this->dbpass"; // debug
        $this->conn = mysqli_connect($this->hostname, $this->dbuser,$this->dbpass);
        if (!$this->conn) {
            $this->mysqli_err = "connect failed: " . mysqli_connect_error();
        } else {
            $this->db = mysqli_select_db($this->conn, $this->dbname);
            if (!$this->db) {
                $this->mysqli_err = "db select failed: " . mysqli_connect_error();
                $this->conn = 0;
            }
        }
        $GLOBALS["conn"] = $this->conn;
        return($this->conn);
    }   // connect()


    //function getEntry($object, $table) {
    function getEntry() {

        $object = get_called_class();
        $table = $this->table_name;

        $row = $this->getRows($table, "id=$this->id");
        if (mysqli_num_rows($row) != 1) {
            $this->error = "oops getEntry($object, $table)[id=" . $this->id . "] failed.";
            return(null);
        }
        $ent = mysqli_fetch_object($row, $object);
        // angeldie("getEntry: returning: " . serialize($ent));   // debug
        return($ent);
    }
    /*
     * return a result as per the input where clause
     */
    function getRows($table, $where) {
        if ($this->conn == NULL)
            $this->connect();

        $sql = "SELECT * FROM $table WHERE $where";
        //if (strpos($where, "LIKE") != false)
        //angeldie($sql);  // debug
        $result = mysqli_query($this->conn, $sql);
        if (!$result) {
            $this->mysqli_err = mysqli_error($this->conn);
            // angeldie($sql . " " . mysqli_error($this->conn));
        }
        return $result;

    }

    /*
  * return a result as per the input where clause
  */
    function getRowsCustom($sql) {
        if ($this->conn == NULL)
            $this->connect();

        //angeldie($sql);  // debug
        $result = mysqli_query($this->conn, $sql);
        if (!$result) {
            $this->mysqli_err = mysqli_error($this->conn);
            // angeldie($sql . " " . mysqli_error($this->conn));
        }
        return $result;

    }

     /********************************************************************************
     * CLEANUP NEEDED
     * I think insertRow and updateRow are the only things we need, but my definitions
     * got confused here. I think the second version of each is better, but we might have
     * backwards compat issues!
     */

     /*
      * add a row to the input table
      */
    function addRow($values) {
        if ($this->conn == NULL)
            $this->connect();

        $table = $this->table_name;
        $columns = $this->column_names;

        $sql = sprintf("INSERT INTO $table (%s) VALUES (%s);", $columns, $values);
        //angeldie($sql);   // debug
        $result = mysqli_query($this->conn, $sql);
        if (!$result) {
            $this->mysqli_err = mysqli_error($this->conn);
            // angeldie($sql . " " . mysqli_error($this->conn));
        }
        return $result;

    }



    /********************************************************************************
     * @param $columnsAndvalues
     * @param $where
     * @return bool|mysqli_result
     */

    function updateRow($columnsAndvalues, $where) {
        if ($this->conn == NULL)
            $this->connect();

        $table = $this->table_name;
        $columns = $this->column_names;

        $sql = sprintf("UPDATE $table SET %s WHERE %s;", $columnsAndvalues, $where);
        // angeldie($sql);   // debug
        // error_log("---updateRow: " . $sql);

        $result = mysqli_query($this->conn, $sql);
        if (!$result) {
            $this->mysqli_err = mysqli_error($this->conn);
            // angeldie($sql . " " . mysqli_error($this->conn));
        }
        return $result;

    }

    function deleteRows($table, $where) {
        if ($this->conn == NULL)
            $this->connect();

        $sql = "DELETE FROM $table WHERE $where";
        // angeldie($sql);                      // debug
        // error_log("deleteRows: " . $sql);   // debug
        $result = mysqli_query($this->conn, $sql);
        if (!$result) {
            $this->mysqli_err = mysqli_error($this->conn);
        }
        return $result;

    }

    /*
     * add a new row to the table and return its id
     */

    function insertRow() {

        $conn = $this->connect();

        $colArr = explode(",", $this->column_names);
        // angeldie("addInvestment:" . serialize($colArr));
        $valueList = "";
        foreach ($colArr as $col) {
            $col = trim($col);  // remove any extraneous white space
            if ($col != "id" and $col != "last_update") {
                if ($valueList != "")
                    $valueList .= ',';
                $val = $this->{$col};
                if (is_string($val))
                    $val = mysqli_escape_string($conn, $val );

                $valueList .= "'" . $val . "'";
            }

        }
        //angeldie($valueList); // debug

        $result = $this->addRow($valueList);

        if ($result == NULL)
            angeldie("addEntry addRow failed: " . $this->error() );

        /*
         * other than for news, store a new entry in the newa_entries table
         */
        if ($this->table_name != "news_table") {
            $this->id = mysqli_insert_id($this->conn);
            $uID = $_SESSION['loggedInUserID'];
            $q = "INSERT INTO new_entries (type,table_id,userID) VALUES ('$this->table_name','$this->id','$uID')";

            // angeldie($q);
            $result = mysqli_query($conn,$q);
        }

        return($this->id);
    }

    /**************************************
     * @return int|string
     */


    function updateThisRow() {

        $conn = $this->connect();

        $colArr = explode(",", $this->column_names);
        // angeldie("updateThisRow:" . serialize($this));    // debug
        $valueList = "";
        foreach ($colArr as $col) {
            $col = trim($col);  // remove any extraneous white space
            if ($col != "id" and $col != "last_update") {
                if ($valueList != "")
                    $valueList .= ',';
                $val = $this->{$col};
                if (is_string($val))
                    $val = mysqli_escape_string($conn, $val );

                $valueList .= $col . "='" . $val . "'";
            }

        }
        // angeldie("updateThisRow: " . $valueList);   // debug

        $result = $this->updateRow($valueList, 'id=' . $this->id);

        if ($result == NULL)
            angeldie("updateThisRow updateRow failed: " . $this->error() );

        return($this->id);
    }

    function error() {
        if ($this->mysqli_err)
            return $this->mysqli_err;
         else
            return "no error found.";

    }


     /*
      * This is a cool function which creates a map for any table of any key to any value. E.g. if you
      * wanted a map of company names to ids, you would call:
      *                                           $db->createMap($companies,'companytable','id','name')
      */
    function createMap(&$map,$table,$fieldLeft,$fieldRight) {

        $con = $GLOBALS["conn"];
        if (!$con)
            $con = $this->connect();

        if (!$con)
            return 'Could not connect: ' . $this->error();

        /*
         * read all the values from the table and create map - order by right side just
         * because that will usually be correct (I think!!)
         */

        $q = sprintf("SELECT %s,%s FROM %s ORDER BY $fieldRight ASC", $fieldLeft, $fieldRight, $table);

        $result = mysqli_query($con, $q);
        if (!$result) {
            return 'createMap: company lookup failed: ' . $q . mysqli_error($con);
        }


        while($row = mysqli_fetch_array($result))
        {
            $leftValue = $row[$fieldLeft];
            $rightValue = $row[$fieldRight];
            $map[$leftValue] = $rightValue;
        }
        return NULL;
    }
    function columns() {

        return($this->column_names);
    }
    function tableName() {

        return($this->table_name);
    }
    function setTableName($name) {

        $this->table_name = $name;
    }

}   // class ik12db

?>
