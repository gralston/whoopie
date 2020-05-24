<?php

    set_include_path(get_include_path() . ":" . "./includes" . ":" . "../stack/php/lib/php" );

    require "angel_misc.inc";
    require "angelObjects.inc";
    require "angelreg.inc";      // controller and view for ik12 registration.  Most real work done here.
    require "angellogin.inc";    // required for login object to create a password hash.
    require "angelinvestments.inc";     // required to save model


    $tr = new angelTracker();
    $p = new angelPerson();
    $p->connect();
    $p->firstname = "Geoff";
    $p->email = "geoff@yahoo.com";

    $investors = readTrackers();

    foreach (array_keys($investors) as $investorID) {

        $p->id = $investorID;
        $p->email = ""; // don't search on email
        $foundp = $p->findMatchingPerson(false);
        // echo $investorID . ": " . $foundp->email . "\n";

        sendTrackerEmail($foundp, $investors[$investorID]);
    }


function readTrackers() {

    $investors = array();

    $tr = new angelTracker();
    $conn = $tr->connect();

    $r = $tr->getRows("trackertable","id != 0");    // get all rows

    if ($r == NULL) {
        error_log("trackerEmail.php: 'getrows (trackertable) failed: " . mysqli_error($conn));
        die();
    }

    while($t = mysqli_fetch_object($r, "angelTracker")) {
        if ($t->status == "live") {
            $investorID = $t->investorID;
            if (array_key_exists($investorID, $investors))
                $investors[$investorID][] = $t;
            else {
                $tarr = array();
                $tarr[] = $t;
                $investors[$investorID] = $tarr;
            }
        }
    }

    return($investors);
}

/***********************
 * this method sends a verify email and confirms that it was sent. It assumes the input person
 * is already in the database with their hash set.
 */
function sendTrackerEmail($p, $trackers) {

    // angeldie(serialize($p));    // debug

    $peopleMap = array();
    $companyMap = array();
    $p->connect();

    $p->createPersonMap($peopleMap);
    $p->createMap($companyMap,"companytable", "id", "name");

    $subject    = 'Time to Reach Out!';
    $body       = "Hey $p->firstname, it would be a really good time to reach out to these companies and people:\n\n";

    foreach ($trackers as $tr) {
        if ($tr->type == "company") {
            $name = $companyMap[$tr->companyID];
        } else {
            $name = $peopleMap[$tr->personID];
        }
        $notes = "";
        if ($tr->notes != "" and $tr->notes != NULL)
            $notes = "  \t (" . stripcslashes($tr->notes) . ")";

        $body .= "\t" . $name . $notes . "\n";
    }

    $body .= "\nAll the best,\nAngelcalc\n";

    // echo $body;
    angelSendEmail($subject, $body, $p->firstname,$p->email);

}
?>