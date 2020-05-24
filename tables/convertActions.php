<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 6/1/14
 * Time: 1:30 PM
 */

set_include_path(get_include_path() . ":" . "./includes" . ":" . "../includes" );

require "angelObjects.inc";
require "angel_misc.inc";


$tableName = 'converttable';
$companyTOid = array();


angelAuthenticate("root");  /* a bit weird here, but I think it will do the right thing */


try
{

    //Open database connection

    $mod = new angelConvert();
    $con = $mod->connect();
    if (!$con)
        throw new Exception('Could not connect: ' . $mod->error());

    //Getting records (listAction)
    if($_GET["action"] == "list") {
        angellog("Convert table: list");
        // throw new Exception('under development list' . serialize($_REQUEST));

        /*
         * Start by applying any filters.  Currently support a substring search on name, decision, cohort
         * all implement using a mysql WHERE clause
        */

        $needWhere = true;
        $wherePart = "";
        if ($_POST['model_id'] != NULL) {
            $n = $_POST['model_id'];

            $wherePart = sprintf(" WHERE model_id=%s", $n );
            $needWhere = false;
        }



        /*
         * Get record count - as limited by our filters ($wherePart)
         */
        $result = mysqli_query($con, "SELECT COUNT(*) AS RecordCount FROM " . $tableName . $wherePart);
        $row = mysqli_fetch_array($result);
        $totalNumRows = $row['RecordCount'];

        /*
         * Get records from database
         */

        $q = sprintf("SELECT * FROM %s %s ORDER BY %s LIMIT %s,%s", $tableName, $wherePart, $_GET["jtSorting"],  $_GET["jtStartIndex"], $_GET["jtPageSize"] );

        $result = mysqli_query($con, $q);
        if (!$result) {
            throw new Exception('list query failed: ' . $q . mysqli_error($con));
        }


        //throw new Exception('query succeeded - found this many rows: ' . $NumRows);
        //Add all records to an array
        $rows = array();
        $i = 0;
        while($row = mysqli_fetch_array($result) and $i++ < 10000)
        {
            $rows[] = $row;
        }

        //Return result to jTable
        $jTableResult = array();
        $jTableResult['Result'] = "OK";
        $jTableResult['Records'] = $rows;
        $jTableResult['TotalRecordCount'] = $totalNumRows;

        print json_encode($jTableResult);
    }
    //Creating a new record (createAction)
    else if($_GET["action"] == "create") {
        // throw new Exception('under development create');

        $keys = $mod->columns();       // get list of table column names

        $updateQ = "";
        $needComma = false;
        foreach ($keys as $k) {
            if (array_key_exists($k, $_POST)) {
                if ($needComma) {
                    $updateQ .= ",";
                }

                if ($k != "id" and $k != "last_update") {
                    $updateQ .= "'" . mysqli_escape_string($con, $_POST[$k]) . "'";
                    $needComma = true;
                }
            }

        };

        angellog("Convert table: create entry: " . mysqli_escape_string($con, $updateQ));


        $result = $mod->addRow($updateQ);           // Insert record into database
        if (!$result) {
            throw new Exception('addRow failed: ' . mysqli_error($con));
        }

        //Get last inserted record (to return to jTable)
        $q =  sprintf("SELECT * FROM %s WHERE id = LAST_INSERT_ID();", $tableName);
        $result = mysqli_query($con, $q);
        if (!$result) {
            throw new Exception('create query (select) failed: ' . $q . mysqli_error($con));
        }
        $row = mysqli_fetch_array($result);


        //Return result to jTable
        $jTableResult = array();
        $jTableResult['Result'] = "OK";
        $jTableResult['Record'] = $row;
        print json_encode($jTableResult);
    }

    //Updating a record (updateAction)
    else if($_GET["action"] == "update")
    {

        $keys = array_keys(get_object_vars($mod));

        $updateQ = "";
        $needComma = false;
        foreach ($keys as $k) {
            if (array_key_exists($k, $_POST)) {
                if ($k != "id" and $k != "last_update") {   // never change id - it is the key - and we'll update date below

                        if ($needComma)
                            $updateQ .= ", ";

                        $updateQ .= $k . "='" . mysqli_escape_string($con, $_POST[$k]) . "'";
                        $needComma = true;
                    }
            }
        };
        $dt = new DateTime();
        $updateQ .= ", last_update='" . $dt->format('Y-m-d H:i:s') . "'";

        //Update record in database
        $q =  sprintf("UPDATE %s SET %s WHERE id=%s",$tableName, $updateQ, $_POST["id"] );
        // throw new Exception('under development update: ' . $q);

        $result = mysqli_query($con, $q);
        if (!$result) {
            throw new Exception('update query failed: ' . $q . mysqli_error($con));
        }
        angellog("Convert table: update entry: " . mysqli_escape_string($con, $updateQ));
        //Return result to jTable
        $jTableResult = array();
        $jTableResult['Result'] = "OK";
        print json_encode($jTableResult);
    }
    //Deleting a record (deleteAction)
    else if($_GET["action"] == "delete")
    {
        // throw new Exception('under development delete');
        // Delete from database
        $q =  sprintf("DELETE FROM %s WHERE id=%s",$tableName, $_POST["id"] );
        // throw new Exception('under development delete: ' . $q);

        $result = mysqli_query($con, $q);
        if (!$result) {
            throw new Exception('delete query failed: ' . $q . mysqli_error($con));
        }
        angellog("Convert table: delete entry: id=" . $_POST["id"]);
        //$result = mysql_query("DELETE FROM people WHERE PersonId = " . $_POST["PersonId"] . ";");

        //Return result to jTable
        $jTableResult = array();
        $jTableResult['Result'] = "OK";
        print json_encode($jTableResult);
    }

    //Close database connection
    mysqli_close($con);

}
catch(Exception $ex)
{
    //Return error message
    $jTableResult = array();
    $jTableResult['Result'] = "ERROR";
    $jTableResult['Message'] = $ex->getMessage();
    print json_encode($jTableResult);
}



