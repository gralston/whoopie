<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 1/21/17
 * Time: 10:35 AM
 */

class angelBasis {
    public $queryString = "";
    public $map = array();


    function qsToMap() {
        parse_str($this->queryString, $this->map);
    }

    function mapToQS() {
        $qs = "";
        $needAmpersand = false;
        foreach (array_keys($this->map) as $id) {
            if ($this->map[$id] != 0) {         // if basis is 0, don't bother including it
                if ($needAmpersand)
                    $qs .= "&$id=" . $this->map[$id];
                else {
                    $qs .= "$id=" . $this->map[$id];
                    $needAmpersand = true;
                }
            }
        }
        $this->queryString = $qs;
    }


    function basis($id) {
        $this->qsToMap();   // always make sure map is up to date
        if ($this->map[$id] == "")
            return(0);

        return($this->map[$id]);
    }

    function totalBasis() {
        $this->qsToMap();   // always make sure map is up to date
        $total = 0;
        foreach (array_keys($this->map) as $id)
            if ($this->map[$id] != "")
                $total += $this->map[$id];
        return($total);

    }

    function idToBasis($id) {
        $this->qsToMap();   // always make sure map is up to date
        return($this->map[$id]);
    }

    function updateBasis($id, $basis) {
        $this->qsToMap();   // always make sure map is up to date
        $this->map[$id] = $basis;
        $this->mapToQS();   // update  query string is up to date
    }

    function updateBasisDB($table, $id, $queryString) {
        $db = new angeldb();
        $db->connect();

        $db->id = $id;
        $db->setTableName($table);

        $db->updateRow("basis=$queryString", "id=$id");
    }

} // angelObjBasis