<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 12/10/14
 * Time: 11:36 PM
 */


    set_include_path(get_include_path() . ":" . "./includes" );

    require_once "angelObjects.inc";
    require_once "angellogs.inc";   // controller and view for logs
    require_once "angel_misc.inc";

    $auth = angelAuthenticate("root"); /* check to make sure we are logged in */
    /*
     * This page is called:
     *  1) to display a default view of ik12 logs (just recent logs) ($day is NULL)
     *  2) to display a view of ik12 logs for a particular day
     */

    $log = new angellogs();

    //angeldie(serialize($_POST));

    if (empty($_POST) and empty($_GET)) {                       /* Case (1) */
        $log->showLogsPage(NULL, NULL);
    } else {                                                    /* Case (2) */
        if (array_key_exists("day", $_POST)) {
            $day = $_POST['day'];
        } elseif (array_key_exists("day", $_GET)) {
            $day = $_GET['day'];
        } else
            $day = NULL;

        if (array_key_exists("logSearch", $_POST))
            $search = $_POST["logSearch"];
        else
            $search = NULL;

        if ($day == 'today' or $day == 'yesterday')
            $day = date('Y-m-d', strtotime($day));
            // $day = date('Y-m-d');
        else
            $day = date('Y-m-d', strtotime(str_replace('-', '/', $day)));       // convert to mysql format

        // ik12die($day);           // debug
        $log->showLogsPage($day, $search);

    }
?>


