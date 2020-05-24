<?php
/*
 * Object for company news
 */
 
class angelNews extends angeldb {

    public $companyID       = 0;        // if there is a relevant company
    public $type            = "";       // news type: currently unused
    public $newsDate        = "";       // date story published
    public $source          = "";       // source of the story
    public $title          = "";        // title of the story
    public $url             = "";       // url of the story

    function __construct() {
        parent::__construct();

        $this->column_names = "companyID,type,newsDate,source,title,url";
        $this->table_name = "newstable";
    }

    function updateSavedNews($investorID) {

        $news = $this->getAllNewsfromAPI($investorID);
        // angeldie(serialize($news));
        //for ($j = 0; $j < sizeof($news); $j++ ) {
        foreach (array_keys($news) as $coID ) {
            $companyNews = $news[$coID];
            //angeldie(serialize($companyNews));

            $results = $companyNews["totalResults"];
            if ($results > 0)
                $results = 1;   // one result per company

            for ($i=0;$i < $results; $i++) {

                $this->companyID = $coID;
                $this->source = $companyNews["articles"][$i]["source"]["name"];
                $this->title = $companyNews["articles"][$i]["title"];
                $this->url = $companyNews["articles"][$i]["url"];

                $this->newsDate = substr($companyNews["articles"][$i]["publishedAt"], 0, 10);
                // angeldie("yyy: " . serialize($this));

                if ($this->source == "" or $this->title == "" or $this->url == "")
                    continue;        // skip empty stories
                else
                    $this->addOrUpdateNews();

            }
        }
    }

    function getAllNewsfromAPI($investorID) {
        $allNews = array();
        $cos = array();


        angelGetAllMyActiveCompanies($cos, $investorID);
        // angeldie(serialize($cos));

        for ($i=0; $i < sizeof($cos); $i++) {
        // for ($i=0; $i < 20; $i++) {
            $co = $cos[$i];
            // angeldie($co["name"]);
            $news = $this->getCompanyNews($co["name"]);
            $allNews[$co["id"]] = $news;
        }

        return($allNews);


    }   // getAllNewsfromAPI

    function getAllNewsfromDB($investorID) {

        $news = new angelNews();

        $rows = $news->getRows($news->table_name, "id != 0 ORDER by newsDate DESC");

        return($rows);

    }   // getAllNewsfromDB


    function getCompanyNews($companyName) {

        $companyName = urlencode($companyName);
        $today = date("Y-m-d");
        // angeldie($today . ": getting news for: " . $companyName);
        $APIKEY = "d1e584a164314677a933b1cdd3e7e919";
        $sources = "techcrunch,the-verge,bloomberg,buzzfeed,recode,the-next-web,hacker-news";
        $newsURL = "http://newsapi.org/v2/everything?q=$companyName&from=$today&to=$today&language=en&sortBy=relevancy&sources=$sources&apiKey=$APIKEY";
        // angeldie($newsURL);



        $curl = curl_init();

        curl_setopt_array($curl, array(
            CURLOPT_URL => $newsURL,
            CURLOPT_RETURNTRANSFER => true,
            CURLOPT_TIMEOUT => 30,
            CURLOPT_HTTP_VERSION => CURL_HTTP_VERSION_1_1,
            CURLOPT_CUSTOMREQUEST => "GET",
            CURLOPT_HTTPHEADER => array(
                "cache-control: no-cache"
            ),
        ));

        $response = curl_exec($curl);
        $err = curl_error($curl);

        if ($err != "") {
            angeldie(  "Curl Error: " . $err);
        }

        curl_close($curl);

        $response = json_decode($response, true); //because of true, it's in an array

        $status = $response["status"];
        if ($status == "error") {
            $err = $status = $response["message"];
            angeldie("newsapi Error: " . $err);
        }

        return($response);
    }   // getCompanyNews()

    function addOrUpdateNews() {

        $news = $this->findNewsEntry();
        if ($news == NULL)
            $this->insertRow();
        else {
            // angeldie("updating news for company: " . $this->companyID);
            $this->id = $news->id;
            $this->updateThisRow();
        }
    }

    function findNewsEntry() {

        if ($this->conn == null)
            $this->connect();

        $rows = NULL;
        if ($this->companyID != 0) {
            $id =   $this->companyID;
            $rows = $this->getRows("newstable", "companyID=$id");
        }
        if ($rows == NULL)
            $numrows = 0;
        else
            $numrows = mysqli_num_rows($rows);

        if ($numrows == 0) {
            /*
             * no name match, now try other_names (not yet implemented)
             */

        } else if ($numrows == 1) {
            $existingNews = mysqli_fetch_object($rows,"angelNews");
            // angeldie("uh oh: $this->name");
            return $existingNews;
        }

        return NULL;

    }

}  // angelNews