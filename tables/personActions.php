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


$tableName = 'persontable';
$companyTOid = array();

angelAuthenticate("root");  /* a bit weird here, but I think it will do the right thing */


try
{

    //Open database connection

    $p = new angelPerson();
    $con = $p->connect();
    if (!$con)
        throw new Exception('Could not connect: ' . $p->error());

    //Getting records (listAction)
    if($_GET["action"] == "list") {
        angellog("People table: list");
        // throw new Exception('under development list' . serialize($_REQUEST));

        /*
         * Start by applying any filters.  Currently support a substring search on name, decision, cohort
         * all implement using a mysql WHERE clause
        */

        $needWhere = true;
        $wherePart = "";
        if ($_POST['name'] != NULL) {
            $n = $_POST['name'];

            $wherePart = sprintf(" WHERE (firstname LIKE '%%%s%%' OR lastname LIKE '%%%s%%')", $n, $n );
            $needWhere = false;
        }
        if ($_POST['email'] != NULL) {
            $e = $_POST['email'];

            $wherePart = sprintf(" WHERE email LIKE '%%%s%%' ", $e );
            $needWhere = false;
        }
        if ($_POST['id'] != NULL) {
            $n = $_POST['id'];
            if ($needWhere) {
                $wherePart .= " WHERE id='" . $n . "'";
                $needWhere = false;
            } else
                $wherePart .= " AND id='" . $n. "'";
        }
        if ($_POST['companyName'] != NULL) {
            $n = $_POST['companyName'];
            if ($err = $p->createMap($companyTOid,'companytable','name','id'))
                throw new Exception($err);
            // throw new Exception(serialize($companyTOid));
            if (array_key_exists($n, $companyTOid)) {
                $id = $companyTOid[$n];
                $wherePart .= " WHERE company_id='" . $id . "'";
                $needWhere = false;
            }

        }
        if ($_POST['kind'] != NULL) {
            $n = $_POST['kind'];
            if ($needWhere) {
                $wherePart .= " WHERE kind='" . $n . "'";
                $needWhere = false;
            } else
                $wherePart .= " AND kind='" . $n. "'";
        }
        if ($_POST['gender'] != NULL) {
            $n = $_POST['gender'];
            if ($needWhere) {
                $wherePart .= " WHERE gender='" . $n . "'";
                $needWhere = false;
            } else
                $wherePart .= " AND gender='" . $n. "'";
        }
        /*
        if ($_POST['status'] != NULL) {
            $n = $_POST['status'];
            if ($needWhere) {
                $wherePart .= " WHERE status='" . $n . "'";
                $needWhere = false;
            } else
                $wherePart .= " AND status='" . $n. "'";
        }
       if ($_POST['cohorts'] != NULL) {
            $n = $_POST['cohorts'];
            if ($needWhere) {
                $wherePart = sprintf(" WHERE cohorts LIKE '%%%s%%'", $n);
                $needWhere = false;
            } else {
                $wherePart .= sprintf(" AND cohorts LIKE '%%%s%%'", $n);
            }
        }
  */
        if ($_POST['dday_attend'] != NULL) {
            $n = $_POST['dday_attend'];
            if ($needWhere) {
                $wherePart = sprintf(" WHERE dday_attend LIKE '%%%s%%'", $n);
                $needWhere = false;
            } else {
                $wherePart .= sprintf(" AND dday_attend LIKE '%%%s%%'", $n);
            }
        }
        if ($_POST['eday_attend'] != NULL) {
            $n = $_POST['eday_attend'];
            if ($needWhere) {
                $wherePart = sprintf(" WHERE eday_attend LIKE '%%%s%%'", $n);
                $needWhere = false;
            } else {
                $wherePart .= sprintf(" AND eday_attend LIKE '%%%s%%'", $n);
            }
        }
        /*
         * the following two filters are checkboxes which return 'true' if checked 'false' otherwise
         */
        $n = $_POST['invite_to_dday'];
        if ($n == 'true') {
            if ($needWhere) {
                $wherePart = " WHERE invite_to_dday = (1)";
                $needWhere = false;
            } else {
                $wherePart .= " AND invite_to_dday = (1)";
            }
        }
        $n = $_POST['invite_to_eday'];
        if ($n == 'true') {
            if ($needWhere) {
                $wherePart = " WHERE invite_to_eday = (1)";
                $needWhere = false;
            } else {
                $wherePart .= " AND invite_to_eday = (1)";
            }
        }




        // throw new Exception('testing: ' . serialize($_REQUEST)); // debug
        /*
         * Get record count - as limited by our filters ($wherePart)
         */
        $result = mysqli_query($con, "SELECT COUNT(*) AS RecordCount FROM " . $tableName . $wherePart);
        $row = mysqli_fetch_array($result);
        $totalNumRows = $row['RecordCount'];

        /*
         * Get records from database
         */
        // throw new Exception($_GET["jtSorting"]);
        if (strpos($_GET["jtSorting"],"models") === false and strpos($_GET["jtSorting"],"invests") === false ) {
            /*
             * standard query for one of the columns
             */
            $q = sprintf("SELECT * FROM %s %s ORDER BY %s LIMIT %s,%s", $tableName, $wherePart, $_GET["jtSorting"],  $_GET["jtStartIndex"], $_GET["jtPageSize"] );
        } else {
            /*
             * this is a query with a sort of models or investments which require a join to sort
             */
            //throw new Exception($_GET["jtSorting"]);  // debug

            if (strpos($_GET["jtSorting"],"ASC") === false )
                $direction= "DESC";
            else
                $direction = "ASC";
            
            if (strpos($_GET["jtSorting"],"models") === false) {
                $q = sprintf("SELECT p.*, COUNT(a.id) as count  FROM persontable  p LEFT JOIN angelinvests a ON p.id = a.investor_id GROUP BY p.id ORDER by count %s LIMIT %s,%s",
                    $direction, $_GET["jtStartIndex"], $_GET["jtPageSize"]);
            } else {
                $q = sprintf("SELECT p.*, COUNT(m.id) as count  FROM persontable  p LEFT JOIN modeltable m ON p.id = m.investor_id GROUP BY p.id ORDER by count %s LIMIT %s,%s",
                                                $direction, $_GET["jtStartIndex"], $_GET["jtPageSize"]);
            }
            // throw new Exception('list query: ' . $q);
        }

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

        $keys = array_keys(get_object_vars($p));       // get list of fields in table

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

        angellog("People table: create entry: " . mysqli_escape_string($con, $updateQ));
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

        //$keys = preg_split("/[\s,]+/", $fieldList);
        $keys = array_keys(get_object_vars($p));

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
        angellog("People table: update entry: " . mysqli_escape_string($con, $updateQ));
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
        angellog("People table: delete entry: id=" . $_POST["id"]);
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



