<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 5/21/14
 * Time: 8:11 PM
 *
 * This script is run to respond to a client side database query via jtable.  The results are packaged up by jtable
 * into a json encoded file and returned.  Nothing displayable is output by this script.
 */

set_include_path(get_include_path() . ":" . "./includes" . ":" . "../includes" );

require "angelObjects.inc";
require "angel_misc.inc";

angelAuthenticate("root");  /* a bit weird here, but I think it will do the right thing */


$tableName = 'companytable';

/*
$companyFields = array("id","last_update","name","description","location","url","founder_ids","ik12_relation",
    "ik12_decision","app_reference","status","yr_founded","articles","accelerators","tags");

$fieldList = "id,last_update,name,location,description,ik12_relation,url,ik12_decision,founder_ids,tags,yr_founded";
*/

try
{

    //Open database connection

    $co = new angelCompany();
    $con = $co->connect();
    if (!$con)
        throw new Exception('Could not connect: ' . $co->error());

    //Getting records (listAction)
    if($_GET["action"] == "list") {
        angellog("Company table: list");
        // throw new Exception('under development list' . serialize($_REQUEST));

        /*
         * Start by applying any filters.  Currently support a substring search on name, decision, cohort
         * all implement using a mysql WHERE clause
        */

        $needWhere = true;
        $wherePart = "";
        if ($_POST['name'] != NULL) {
            $n = $_POST['name'];
            $wherePart .= " WHERE name LIKE '%" . $n . "%'";
            $needWhere = false;
        }
        if ($_POST['decision'] != NULL) {
            $n = $_POST['decision'];
            if ($needWhere) {
                $wherePart .= " WHERE decision='" . $n . "'";
                $needWhere = false;
            } else
                $wherePart .= " AND decision='" . $n. "'";
        }

        if ($_POST['kind'] != NULL) {
            $n = $_POST['kind'];
            if ($needWhere) {
                $wherePart .= " WHERE kind='" . $n . "'";
                $needWhere = false;
            } else
                $wherePart .= " AND kind='" . $n. "'";
        }
        if ($_POST['status'] != NULL) {
            $n = $_POST['status'];
            if ($needWhere) {
                $wherePart .= " WHERE status='" . $n . "'";
                $needWhere = false;
            } else
                $wherePart .= " AND status='" . $n. "'";
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

        // $keys = preg_split("/[\s,]+/", $fieldList);
        $keys = array_keys(get_object_vars($co));       // get list of fields in table

        $updateQ = "";
        $insertList = "";
        $needComma = false;
        foreach ($keys as $k) {
            if (array_key_exists($k, $_POST)) {
               if ($needComma) {
                    $updateQ .= ",";
                    $insertList .= ",";
               }

               if ($k != "id" and $k != "last_update") {
                   $insertList .= $k;
                   $updateQ .= "'" . mysqli_escape_string($con, $_POST[$k]) . "'";
                   $needComma = true;
               }
            }
        };
        angellog("Company table: create entry: " . mysqli_escape_string($con, $updateQ));

        // Insert record into database
        $q =  sprintf("INSERT INTO %s(%s) VALUES(%s)",$tableName, $insertList, $updateQ);
        // throw new Exception('under development create: ' . $q);

        $result = mysqli_query($con, $q);
        if (!$result) {
            throw new Exception('create query failed: ' . $q . mysqli_error($con));
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

        // $keys = preg_split("/[\s,]+/", $fieldList);
        $keys = array_keys(get_object_vars($co));       // get list of fields in table

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
        // angellog("Company table: update entry: " . serialize($co));      // debug
        angellog("Company table: update entry: " . mysqli_escape_string($con, $updateQ));
        //Update record in database
        $q =  sprintf("UPDATE %s SET %s WHERE id=%s",$tableName, $updateQ, $_POST["id"] );
        //throw new Exception('under development update: ' . $q);

        $result = mysqli_query($con, $q);
        if (!$result) {
            throw new Exception('update query failed: ' . $q . mysqli_error($con));
        }

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
        angellog("Company table: delete entry: id=" . $_POST["id"]);

        $q =  sprintf("DELETE FROM %s WHERE id=%s",$tableName, $_POST["id"] );
        // throw new Exception('under development delete: ' . $q);

        $result = mysqli_query($con, $q);
        if (!$result) {
            throw new Exception('delete query failed: ' . $q . mysqli_error($con));
        }

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

function uniqueID($con, $table)
{

    $rows = 1;
    $id = 0;
    while ($rows != 0) {
        $id = uniqid();
        $q = sprintf("SELECT id FROM %s WHERE id='%s'", $table, $id);
        $result = mysqli_query($con, $q);
        if (!$result) {
            throw new Exception('uniqueID failed: ' . $q . mysqli_error($con));
        }
        $rows = mysqli_num_rows($result);
    }

    return $id;
}


?>