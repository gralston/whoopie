<?php
/**
 * Created by PhpStorm.
 * User: geoff
 * Date: 6/2/14
 * Time: 8:56 AM
 */



class angelPerson extends angeldb {

    public $creatorID      = 0;    // id of investor who added this founder (if a founder)
    public $firstname       = "";
    public $lastname        = "";
    public $email           = "";
    public $gender          = "";   // m for male, f for female, and o for other
    public $altemail        = "";
    public $adminemail      = "";
    // online personas - mostly for
    public $linkedin        = "";
    public $twitter         = "";
    public $github          = "";
    public $facebook        = "";
    public $website         = ""; // personal website
    //*****************************************************
    public $password        = "";
    public $regdate         = "";   // date of registration
    public $firstLoginDate  = "";
    public $lastLoginDate   = "";
    public $cell            = "";
    public $photo           = "";
    public $dob             = "";
    // ************ bio stuff **********************
    public $bio             = "";   // summary bio
    public $work            = "";   // work history
    public $edu             = "";   // education history
    public $edu_interest    = "";   // interest in education
    public $impressive      = "";   // most impressive thing they have done
    // ****************************************************
    public $kind            = "";       // founder, acperson, user, root (id should be 0)
    public $former_ik12founder = false;
    public $is_an_angel     = false;    // true if the person is an angel even though they are another kind
    public $company_id      = 0;
    public $role            = "";
    public $apps            = "";
    public $access          = "";
    public $hash            = "";   // unique hash for this account created during reg.
    public $state           = "";   // 'static','reg','waitverify', 'verified','partner'
    public $description     = "";

    public $friends         = "";  // comma separated list of friend id's
    public $advisors        = "";  // query string of advisorID=access, where access is (currently) R(read only) or W(read/write)
    public $isAdvisor       = 0;    // if 0, this person is not advising anyone, otherwise they are and will get the advisor start page


 function __construct() {
     parent::__construct();
     $this->table_name = "persontable";
     $this->column_names = 'creatorID,firstname,lastname,email,gender,altemail,adminemail,linkedin,twitter,github,facebook,website,bio,password,regdate,firstLoginDate,lastLoginDate,cell,photo,dob,work,edu,edu_interest,impressive,kind,former_ik12founder,is_an_angel,company_id,role,apps,access,hash,state,description,friends,advisors,isAdvisor';
 }

/*
 * Format a person object and return a string suitable for HTML output.
 */
function htmlPerson(){
    $html = '<div id="htmlPerson"><table id="htmlPersonTable">
              <tr><td>%s %s</td><td>%s</td></tr>
              <tr><td>%s</td><td></td></tr>
             </table></div>';
    $html = sprintf($html, $this->firstname,$this->lastname,$this->email,$this->cell);

    return $html;

}

function findMatchingPerson($emailOnly) {

    if ($this->email != "") {
        $email = mysqli_escape_string($this->conn, $this->email);
        $rows = $this->getRows("persontable", "email='$email'");
    } else if ($this->id == 0)
            return NULL;        // no email, no id, no person
    else {
        $id =   $this->id;
        $rows = $this->getRows("persontable", "id=$id");
    }
    $numrows = mysqli_num_rows($rows);

    if ($numrows == 0) {
        /*
         * no email match, now try with first and last name, unless we are not to try
         */
        if ($emailOnly)
            return NULL;

        $fname = $this->firstname;
        $rows = $this->getRows("persontable", "firstname='$fname'");

        while ($existingPerson = mysqli_fetch_object($rows,"angelPerson")) {
            if ($existingPerson->lastname == $this->lastname) {
                if ($existingPerson->company_id == 0 or $existingPerson->company_id == $this->company_id)
                    return $existingPerson;
            }

        }

    } else {
        $existingPerson = mysqli_fetch_object($rows,"angelPerson");
        // echo "findMatchinePerson: " . "$existingPerson->email" . "\n";   // debug
        return $existingPerson;
    }

    return NULL;

}

    function createPersonMap(&$map) {

        $conn = $this->conn;
        $query = 'SELECT id,firstname,lastname from persontable ORDER BY lastname';

        $result = mysqli_query($conn, $query);
        if (!$result)
            ik12die('createPersonMap: person lookup failed: ' . $query . mysqli_error($conn));

        while($row = mysqli_fetch_array($result)) {
                $map[$row['id']] = $row['firstname'] . " " . $row['lastname'];
        }
        // ik12die(serialize($map));    // debug
    }

	function retrievePerson($conn,$id) {
        $query = "SELECT * from persontable WHERE id='$id'";
        $result = mysqli_query($conn, $query) or ik12die('person lookup failed: ' . $query . mysqli_error($conn));
        $row = mysqli_fetch_array($result);
		extract($row);
    	$this->id              = $id;
    	$this->last_update     = $last_update;
    	$this->firstname       = $firstname;
    	$this->lastname        = $lastname;
    	$this->email           = $email;
    	$this->gender          = $gender;
    	$this->altemail        = $altemail;
    	$this->adminemail      = $adminemail;
    	$this->linkedin        = $linkedin;
    	$this->twitter         = $twitter;
    	$this->regdate         = $regdate;   // date of registration
    	$this->cell            = $cell;
    	$this->photo           = $photo;
    	$this->dob             = $dob;
    	$this->kind            = $kind;   // founder, ik12person, angel, vcperson, educator
    	$this->company_id      = $company_id;
    	$this->role            = $role;
    	$this->apps            = $apps;
    	$this->access          = $access;
    	$this->state           = $state;   // 'static','reg','waitverify', 'verified','partner'
    	$this->description     = $description;
//    	$this->dday_blacklist  = $dday_blacklist; // no more dd invites
//    	$this->eday_blacklist  = $eday_blacklist; // no more ed invites
	}
}  // person