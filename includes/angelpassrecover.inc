<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 12/4/14
 * Time: 10:17 PM
 */

/*
 * angelpassrecover:  controller class for password recovery.  this file also contains the corresponding view.
 *
 * Class which displays password recovery screens, sends a recovery email and allows the user to change their pass.
 */
class angelpassrecover {

    public $view;

    function __construct() {
        $this->view = new angelpassrecover_view();
    }

    /*
     * This method shows the initial recovery page requesting the email address of the account.  If called
     * with a non empty error, then the error is displayed with the recovery form.
     */
    function showRecoverPage($error) {

        $body = $this->view->recoverPageBody($error);
        $this->view->outputHTML($body);
    }

    /*
     * this method actually sends the recovery email and confirms that it was sent.  An error may
     * occur if the email address is not a valid one.
     */
    function sendRecoverEmail($email) {
        $p = new angelPerson();
        $conn = $p->connect();
        if (!$conn) {
            angeldie($p->error());
        }

        /*
         * If this email doesn't exist give an error and reprint the form
         */
        $p->email = $email;

        $existingPerson = $p->findMatchingPerson(true);

        if ($existingPerson == NULL or $existingPerson->password == "" ) {
            $this->showRecoverPage("We cannot find an account with that email address. Please try again.");
        } else {
            /*
             * Generate hash and store it in the person table. That hash will also be embedded in the reset
             * email and used to verify that user returning via the link in the email.
             */
            $existingPerson->hash = md5( rand(0,10000) );             // Generate random 32 character hash
            $q = sprintf("UPDATE persontable SET hash='%s' WHERE id=%s", $existingPerson->hash, $existingPerson->id);
            // angeldie($q);  // debug
            $r = mysqli_query($conn, $q);
            if (!$r)
                angeldie("password recovery hash UPDATE Failed: " . mysqli_error($conn));

            $this->resetEmail($existingPerson->id, $existingPerson->firstname,$existingPerson->email,$existingPerson->hash);
            $body = $this->view->recoveryEmailSentBody();
            $this->view->outputHTML($body);
        }

    }


    /*
     * This method is invoked when the password recovery linked within an email is clicked. Display
     * a form requesting the new password.
     */
    function showNewPassPage($id,$hash) {
        $p = new angelPerson();
        $p->id   = $id;
        $p->hash   = $hash;     // this makes it harder to hack and reset someones password

        $conn = $p->connect();
        if (!$conn) {
            angeldie("resetpass connect error: " . $p->error());
        }

        /*
         * Does an entry exist with this id?
         */
        $existingPerson = $p->findMatchingPerson(true);
        if ($existingPerson == NULL)
            angeldie("Could not find the person with id: " . $p->id);

        if ($existingPerson->hash != $p->hash) {
            angelerror("Your password reset failed. Please contact support.");
        }
        /*
         * This is the correct person. Write the new password form.
         */

        $body = $this->view->newPassPageBody($id,$hash);
        $this->view->outputHTML($body);

    }

    function updatePassword($id,$hash,$pass) {
        $p = new angelPerson();
        $p->id   = $id;
        $p->hash   = $hash;     // this makes it harder to hack and reset someones password

        $path = $GLOBALS["path"];
        $conn = $p->connect();
        if (!$conn) {
            angeldie("resetpass connect error: " . $p->error());
        }

        /*
         * Does an entry exist with this id?
         */
        $existingPerson = $p->findMatchingPerson(true);
        if ($existingPerson == NULL)
            angeldie("Could not find the person with id: " . $p->id);

        if ($existingPerson->hash != $p->hash) {
            angelerror("Your password reset failed (2). Please contact support.");
        }
        /*
         * This is the correct person. Hash the password and write the db with the new password and send them to login.
         */
        $log = new angellogin();
        $p->password = $log->password_hash($pass);     /* store the hashed password */
        if ($p->password == NULL)
            angeldie("Password hash failed!");

        $q = sprintf("UPDATE persontable SET password='%s' WHERE id=%s", $p->password, $p->id);
        // angeldie($q);  // debug
        $r = mysqli_query($conn, $q);
        if (!$r)
            angeldie("password recover pass UPDATE Failed: " . mysqli_error($conn));

        angellog("successful password update (" . $existingPerson->email . ")");
        header("Location: $path/login?passwordreset=1");
    }

    /*
     * Send an email with a reset link to the input user address with the input hash in the return URL.
     */
    function resetEmail($id, $firstname, $email, $hash) {
        require_once "Mail.php";

        $to         = $email;                  // Send email to our user
        $from       = "noreply@badassangel.com";   // this won't do anything since gmail rewrites it
        $host       = "ssl://smtp.gmail.com:465";
        $username   = 'angelcalc.mailbox@gmail.com';
        $password   = 'edtechk12';
        $subject    = 'Password Reset';

        $greeting   = "Dear $firstname,
    ";

        $path = $GLOBALS["path"];
        $body = $greeting . "
You can reset your password by clicking on the url below.

http://angelcalc.com/passrecover?resetlink=1&id=" . $id . '&hash=' . $hash . "

Sincerely,
Geoff";

        angelSendEmail($subject, $body, $firstname, $email);
        /*$headers = array ('From' => $from, 'To' => $to,'Subject' => $subject);
        $smtp = Mail::factory('smtp',
            array ('host' => $host,
                'auth' => true,
                'username' => $username,
                'password' => $password));

        $mail = $smtp->send($to, $headers, $body);

        if (PEAR::isError($mail)) {
            angeldie($mail->getMessage());
        }*/

    }




}    // class angelpassrecover

/*
 * View for angelpassrecover
 */

class angelpassrecover_view extends angelview {

    function __construct() {

        parent::__construct();

        $path = $GLOBALS["path"];

        $this->htmlHeader = "<title>Password Recovery</title>";
        $this->header = "logoOnlyHeader.inc";

        $this->js .= "<script src='$path/jscript/livevalidation_standalone.js'></script>";
        $this->footer       = "angelfooter.inc";


    }

    function recoverPageBody($error) {

        $path = $GLOBALS["path"];
        $body = "<div id='wrap'>
            <p style='margin-top:10px;margin-bottom:15px;color:#7489fe;font-size:150%;font-family: Verdana, Arial, Helvetica, sans-serif;text-align:center'>Password Recovery</p><p style='margin-bottom:10px;margin-left:auto;margin-right:auto;width:90%'><hr></p><p style='font-size:115%'>Please enter the email address used to create your account.</p>";
        if ($error != "")
            $body .= "<p style='color:#FF0000;font-size:75%; border: 2px solid red;width:50%; margin: 0 0 0 160px'>$error</p>";

        $body .= "<div style='margin: 0 0 0 200px;'>
               <form action='$path/passrecover' method='post'>";

        $body .= '
                  <table><tr><td><label for="email">Email:</label></td>
                <td><div id="regTable">
                        <input type="text" name="email" id="f20" value="" tabindex="3" />
                        <script type="text/javascript">
                            var f20 = new LiveValidation("f20", {validMessage: " "});
                            f20.add( Validate.Presence, {failureMessage: "Please enter your email."} );
                            f20.add(Validate.Email, {failureMessage: "Invalid Email!"} );
                        </script>
                    </div></td> </tr>     <tr><td colspan="4">
                            <input type="submit" class="submit_button" value="Recover Password" tabindex="8"></td></tr>


                  </table>
                </form></div>

                    <div style="text-align:right;font-size:70%;font-family: Verdana, Arial, Helvetica, sans-serif;">
                        <a href="login">Back to Login</a> |
                        <a href="reg">Get an Account!</a>
                    </div></div></body>';

        return $body;
    }

    function recoveryEmailSentBody() {

        $body = "<div id='wrap'>
<p style='margin-top:10px;margin-bottom:15px;color:#7489fe;font-size:150%;font-family: Verdana, Arial, Helvetica, sans-serif;text-align:center'>Password Recovery</p><p style='margin-bottom:10px;margin-left:auto;margin-right:auto;width:90%'><hr></p>
                  <p style='font-size:115%'>A password reset has been sent. Please check your email.</p>
                    <div style='text-align:right;font-size:70%;font-family: Verdana, Arial, Helvetica, sans-serif;'>
                        <a href='login'>Back to Login</a> |
                        <a href='reg'>Get an Account</a>
                    </div></div>";
        return $body;
    }

    function newPassPageBody($id,$hash) {
        $path = $GLOBALS["path"];
        $body = "<div id='wrap'>
              <p style='margin-top:10px;margin-bottom:15px;color:#7489fe;font-size:150%;font-family: Verdana, Arial, Helvetica, sans-serif;text-align:center'>Angelcalc Password Recovery</p><p style='margin-bottom:10px;margin-left:auto;margin-right:auto;width:90%'><hr></p><p style='color:#7489fe;font-size:115%'>Please enter a new password.</p>";
        $body .= "<div style='margin: 0 0 0 200px;'>
               <form action='$path/passrecover' method='post'>
                  <table><tr><td><label for='password'>Password:</label>
                <td> <div id='regTable'>
                        <input type='password' name='password' id='myPasswordField' value='' tabindex='1' />
                    </div></td></tr>
                <tr><td><label for='cell'>Confirm Password:</label></td>
                <td><div id='regTable'>
                        <input type='password' name='passwordconf' id='f19' value='' tabindex='2' />
                        <script type='text/javascript'>
                            var f19 = new LiveValidation('f19', {validMessage: ' '});
                            f19.add( Validate.Presence, {failureMessage: 'Please enter a password.'} );
                            f19.add( Validate.Length,  { minimum: 4, maximum: 255, tooShortMessage:'Passwords must be at least 4 characters!' } );
                            f19.add(Validate.Confirmation, { match: 'myPasswordField', failureMessage: 'Passwords do not match!' } );

                        </script>
                    </div></td></tr>     <tr><td colspan='4'>
                            <input type='hidden' name='resetpass' value='1'/>
                            <input type='hidden' name='id' value='$id'/>
                            <input type='hidden' name='hash' value='$hash'/>
                            <input type='submit' class='submit_button' value='Reset Password' tabindex='8'></td></tr>


                  </table>
                </form></div>

                    <div style='text-align:right;font-size:70%;font-family: Verdana, Arial, Helvetica, sans-serif;'>
                        <a href='/login'>Back to Login</a> |
                        <a href='/reg'>Get an Account!</a>
                    </div></div>";

        return $body;
    }

}