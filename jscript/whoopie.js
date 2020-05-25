/**
 * Created by geoff on 5/18/16
 */
/*
 * a convertible object - models a convertible security, either a safe or a note
 */
var RoundMap = {};              // map a round name to its id


var ConvertList = [];           // list of all of the converts
var CurrentYear;                // will always be the current year, e.g. 2018.
/*
 * saved state for a view - will map to cookies
 */

var AngelView;
var FundsView;
function AngelcalcView(top, tables, initTable) {
    this.top            = top;
    this.display        = initTable;
    this.subDisplay     = "";
    this.tables         = tables;
    this.filter         = "all";
    this.trackerFilter  = "all";
    this.sortCol        = {};
    this.sortDir        = {};
    this.sortProp       = {};
    this.navLinks       = {
                            "filterall": "showAll",
                            "filteractive": "showActive",
                            "filteracquired":"showAcquired",
                            "filteripo":"showIPO",
                            "filterdead":"showDead",
                            "filterfundsall":"fundsShowAll",
                            "filterfundsactive":"fundsShowActive",
                            "filterfundscarry":"fundsShowCarry",
                            "filterfundsclosed":"fundsShowClosed",

                            "investments": "investmentsNav",
                            "payouts": "payoutsNav",
                            "companies": 'companiesNav',
                            "values": 'valuesNav',
                            "dealflow": 'dealflowNav',
                            "funds": 'fundsNav',
                            "fundpayouts": 'fundPayoutsNav',
                            "fundpayments": 'fundPaymentsNav',
                            "fundperformance": 'fundPerformanceNav',
                            "fundcashflows": 'fundCashflowsNav',
                            "tracker": 'trackerNav'
                          };
    this.displays       = { "investments": "investmentsContainer",
                            "payouts": "payoutsContainer",
                            "companies": 'companiesContainer',
                            "values": 'valuesContainer',
                            "dealflow": 'dealflowContainer',
                            "funds": 'fundsContainer',
                            "fundpayouts": 'fundPayoutsContainer',
                            "fundpayments": 'fundPaymentsContainer',
                            "fundperformance": 'fundPerformanceContainer',
                            "fundcashflows": 'fundCashflowsContainer',
                            "tracker": 'trackerContainer'
                          };
    this.tableBuilders  = { "investments": showInvestments,
                            "payouts": showPayouts,
                            "companies": showCompanies,
                            "values": showValues,
                            "dealflow": showDealflow,
                            "funds": showFunds,
                            "fundpayouts": showFundPayouts,
                            "fundpayments": showFundPayments,
                            "fundperformance": showFundPerformance,
                            "fundcashflows": showFundCashflows,
                            "tracker": showTracker
                          };
    this.switchDisplay  = function(display) {
        console.debug("switchDisplay: (current, new)" + angelParens(this.display,display));

        this.display        = display;

        /*
         * I think these should be view local variables, but keep as globals for now -  they are
         * here for when viewing a friend's version.
         */
        Investments = MyInvestments;
        Companies = MyCompanies;
        Payouts = MyPayouts;

        this.updateNavbar();        // update navbar colors to show what is currently displayed

        this.showDisplay();         // show this display
        this.save("display");       // save the new display in the cookie
        showAddForm("");            // hide add forms
        this.updateNavSummary();         // update summary info based on current filter
        if (this.top == "angel")
            showGraphics();             // show graphics. really?
        else
            hideGraphics();
        $("#friendHeaderDiv").css("display", "none");   // make sure the friend header is turned off (here?)
        CurrentView.view = this;         // we are now the current view
        CurrentView.topNav = this.top;

        console.debug("switchDisplay: calling tableBuilder");
        this.tableBuilders[this.display](InvestorID);           // make sure this display's table is built

    };
    this.switchFilter  = function(filter) {
        console.debug("view (top,display)" + angelParens(this.top,this.display) + " switch filter (current,new):" + angelParens(this.filter,filter));
        this.filter = filter;
        AngelTotalsFiltered.filter = filter;
        AngelTotalsFiltered.reset();


        this.updateNavbar();        // update navbar colors to show what is currently displayed
        this.showDisplay();         // show this display
        this.save("filter");       // save the new filter in the cookie
        showAddForm("");            // hide add forms
        this.updateNavSummary();         // update summary info based on current filter
        if (this.top == "angel")
            showGraphics();             // show graphics. really?
        else
            hideGraphics();

        if (this.display == "investments")
            showInvestments(InvestorID);
        else if (this.display == "payouts")
            showPayouts(InvestorID);
        else if (this.display == "companies") {

            if (this.subDisplay == "block" || this.subDisplay == null)
                showCompanies(InvestorID);
            else
                showCompanyListSorted(0, null);
        } else if (this.display == "values")
            showValues(InvestorID);
        else if (this.display == "dealflow")
            showDealflow(InvestorID);
        else if (this.display == "funds")
            showFunds(InvestorID);
        else if (this.display == "fundpayouts")
            showFundPayouts(InvestorID);
        else if (this.display == "fundpayments")
            showFundPayments(InvestorID);
        else if (this.display == "fundperformance")
            showFundPerformance(InvestorID);
        else if (this.display == "fundcashflows")
            showFundCashflows(InvestorID);

    };
    this.saveSort       = function(table,sort,property,direction) {
        this.sortCol[table] = sort;
        this.sortProp[table] = property;
        this.sortDir[table] = direction;
        this.save("sort");
    };
    this.save           = function(cook) {
        console.debug("saving cookies: " + cook);
        name = "angelcalc-"+this.top;
        Cookies.set("angelcalc-top", this.top);
        if (cook == "display" || cook == "all")
            Cookies.set(name+"-display", this.display);
        if (cook == "filter" || cook == "all")
            Cookies.set(name+"-filter", this.filter);
        if (cook == "sort" || cook == "all")
            for (var table in this.sortCol)
                Cookies.set(name+table+"-SortColumn", this.sortCol[table]);
        if (cook == "sort" || cook == "all")
            for (table in this.sortDir)
                Cookies.set(name+table+"-SortDirection", this.sortDir[table]);
        if (cook == "sort" || cook == "all")
            for (table in this.sortProp)
                Cookies.set(name+table+"-SortProperty", this.sortProp[table]);
    };
    this.restore        = function() {
        var cookies = Cookies.get();
        name = "angelcalc-"+this.top;

        // console.debug("debugging angelcalcView: " + angelParens(name, JSON.stringify(cookies)));

        if (name+"-display" in cookies)
            this.display = cookies[name+"-display"];
        if (name+"-filter" in cookies)
            this.filter = cookies[name+"-filter"];
        for (var i = 0; i < this.tables.length; i++) {
            table = this.tables[i];
            if (name+table+"-SortColumn" in cookies)
                this.sortCol[table] = cookies[name+table+"-SortColumn"];
        }
        for (i = 0; i < this.tables.length; i++) {
            table = this.tables[i];
            if (name+table+"-SortDirection" in cookies)
                this.sortDir[table] = cookies[name+table+"-SortDirection"];
        }
        for (i = 0; i < this.tables.length; i++) {
            table = this.tables[i];
            if (name+table+"-SortProperty" in cookies)
                this.sortProp[table] = cookies[name+table+"-SortProperty"];
        }


    };
    this.updateNavbar    = function () {

        /*
         * first clear everything
         */
        for (var link in this.navLinks) {
            $('#'+this.navLinks[link]).css('color','black');
        }

        // then make current link white
        $('#'+this.navLinks[this.display]).css('color','white');
        console.debug("updateNavbar: $(#" + this.navLinks[this.display] + ").css('color','white')" );

        // update filters. don't have clever non-if/then/else solution for this jus tnow :(
        if (this.filter == "all") {
            $("#showAll").css("color","white");
            $("#fundsShowAll").css("color","white");
        } else if (this.filter == "active") {
            $("#showActive").css("color","white");
            $("#fundsShowActive").css("color","white");
        } else if (this.filter == "acquired")
            $("#showAcquired").css("color","white");
        else if (this.filter == "ipo")
            $("#showIPO").css("color","white");
        else if (this.filter == "closed") {
            $("#fundsShowClosed").css("color","white");
        } else if (this.filter == "carry") {
            $("#fundsShowCarry").css("color","white");
        } else if (this.filter == "dead")
            $("#showDead").css("color","white");
    }
    this.updateNavbarOLD     = function() {

        $("#showAll").css("color","black");
        $("#showActive").css("color","black");
        $("#showAcquired").css("color","black");
        $("#showIPO").css("color","black");
        $("#showDead").css("color","black");
        $("#fundsShowAll").css("color","black");
        $("#fundsShowActive").css("color","black");
        $("#fundsShowCarry").css("color","black");
        $("#fundsShowClosed").css("color","black");

        $("#investmentsNav").css("backgroundColor","");
        $("#payoutsNav").css("backgroundColor","");
        $("#companiesNav").css("backgroundColor","");
        $("#valuesNav").css("backgroundColor","");
        $("#dealflowNav").css("backgroundColor","");

        $("#fundsNav").css("backgroundColor","");
        $("#fundPayoutsNav").css("backgroundColor","");
        $("#fundPaymentsNav").css("backgroundColor","");
        $("#fundPerformanceNav").css("backgroundColor","");
        $("#fundCashflowsNav").css("backgroundColor","");


        if (this.display == "investments")
            $("#investmentsNav").css("backgroundColor","white");
        else if (this.display == "payouts")
            $("#payoutsNav").css("backgroundColor","white");
        if (this.display == "companies")
            $("#companiesNav").css("backgroundColor","white");
        if (this.display == "values")
            $("#valuesNav").css("backgroundColor","white");
        if (this.display == "dealflow")
            $("#dealflowNav").css("backgroundColor","white");

        if (this.display == "funds")
            $("#fundsNav").css("backgroundColor","white");
        else if (this.display == "fundpayouts")
            $("#fundPayoutsNav").css("backgroundColor","white");
        if (this.display == "fundpayments")
            $("#fundPaymentsNav").css("backgroundColor","white");
        if (this.display == "fundperformance")
            $("#fundPerformanceNav").css("backgroundColor","white");
        if (this.display == "fundcashflows")
            $("#fundCashflowsNav").css("backgroundColor","white");


        if (this.filter == "all") {
            $("#showAll").css("color","white");
            $("#fundsShowAll").css("color","white");
        } else if (this.filter == "active") {
            $("#showActive").css("color","white");
            $("#fundsShowActive").css("color","white");
        } else if (this.filter == "acquired")
            $("#showAcquired").css("color","white");
        else if (this.filter == "ipo")
            $("#showIPO").css("color","white");
        else if (this.filter == "closed") {
            $("#fundsShowClosed").css("color","white");
        } else if (this.filter == "carry") {
            $("#fundsShowCarry").css("color","white");
        } else if (this.filter == "dead")
            $("#showDead").css("color","white");
    };
    this.clearDisplay   = function() {

        var saveDisplay = this.display;
        this.display = "";
        this.showDisplay();
        this.display = saveDisplay;
    };
    this.showDisplay    = function () {

        /*
         * first hide everything
         */
        for (var disp in this.displays) {
            $('#'+this.displays[disp]).css('display','none');
        }

        // then display this display
        $('#'+this.displays[this.display]).css('display','block');
        console.debug("showDisplay: $(#" + this.displays[this.display] + ").css('display','block')" );

        // sub-displays - probably a cleaner way to handle this. later!
        if (this.display == "fundcashflows") {
            if (this.subDisplay == "")
                this.subDisplay = "actuals";
            fundCashflowsContainer.style.display = "block";
            cashflowChooser(null,this.subDisplay);
        } else if (this.display == "companies") {
            if (this.subDisplay == "")
                this.subDisplay = "block";
            companiesContainer.style.display = "block";
            // companyListChooser(null,this.subDisplay);   // this is not needed since ??
        }

    };
    this.showDisplayOLD    = function () {
        var companiesContainer = document.getElementById('companiesContainer');
        var payoutsContainer = document.getElementById('payoutsContainer');
        var investmentsContainer = document.getElementById('investmentsContainer');
        var valuesContainer = document.getElementById('valuesContainer');
        var dealflowContainer = document.getElementById('dealflowContainer');
        var fundsContainer = document.getElementById('fundsContainer');
        var fundPayoutsContainer = document.getElementById('fundPayoutsContainer');
        var fundPaymentsContainer = document.getElementById('fundPaymentsContainer');
        var fundPerformanceContainer = document.getElementById('fundPerformanceContainer');
        var fundCashflowsContainer = document.getElementById('fundCashflowsContainer');

        investmentsContainer.style.display = "none";
        payoutsContainer.style.display = "none";
        companiesContainer.style.display = "none";
        valuesContainer.style.display = "none";
        dealflowContainer.style.display = "none";
        fundsContainer.style.display = "none";
        fundPayoutsContainer.style.display = "none";
        fundPaymentsContainer.style.display = "none";
        fundPerformanceContainer.style.display = "none";
        fundCashflowsContainer.style.display = "none";

        if (this.display == "investments")
            investmentsContainer.style.display = "block";
        else if (this.display == "payouts")
            payoutsContainer.style.display = "block";
        else if (this.display == "companies") {
            if (this.subDisplay == "")
                this.subDisplay = "block";
            companiesContainer.style.display = "block";
            // companyListChooser(null,this.subDisplay);   // this is not needed since
        } else if (this.display == "values")
            valuesContainer.style.display = "block";
        else if (this.display == "dealflow")
            dealflowContainer.style.display = "block";
        else if (this.display == "funds")
            fundsContainer.style.display = "block";
        else if (this.display == "fundpayouts")
            fundPayoutsContainer.style.display = "block";
        else if (this.display == "fundpayments")
            fundPaymentsContainer.style.display = "block";
        else if (this.display == "fundperformance")
            fundPerformanceContainer.style.display = "block";
        else if (this.display == "fundcashflows") {
            if (this.subDisplay == "")
                this.subDisplay = "actuals";
            fundCashflowsContainer.style.display = "block";
            cashflowChooser(null,this.subDisplay);
        }

    };
    this.updateNavSummary   = function () {
        if (this.top == "angel") {
            var totReturn = calcTotalReturn(this.filter);
            var totCompanies = countCompanies(this.filter);
            var totInvestments = countInvestments(this.filter);
            var totInvested = calcTotalInvested(this.filter);
            var activeCompanies = countCompanies("active");
            var totValue = calcTotalValue();        // for now assume counts active only

            Companies.count = totCompanies;
            Investments.count = totInvestments;

            $("#summaryInvestments").html(totInvestments);
            $("#summaryInvested").html("$" + totInvested.toLocaleString());
            $("#summaryCompanies").html(totCompanies.toLocaleString());
            $("#summaryReturned").html("$" + totReturn.toLocaleString());
            $("#summaryActive").html(activeCompanies.toLocaleString());
            $("#summaryValue").html("$" + totValue.toLocaleString());
        } else {
            var totFunds = countFunds(this.filter);
            var totActiveFunds = countFunds("active");
            var totCommitted = calcTotalFundsCommitted(this.filter);
            var totInvested = calcTotalFundsInvested(this.filter);
            var totCommitLeft = totCommitted - totInvested;
            var totReturned = calcTotalFundsReturn(this.filter);

            $("#summaryFunds").html(totFunds.toLocaleString());
            $("#summaryFundsActive").html(totActiveFunds.toLocaleString());
            $("#summaryFundsCommitted").html("$" + totCommitted.toLocaleString());
            $("#summaryFundsCommitLeft").html("$" + totCommitLeft.toLocaleString());
            $("#summaryFundsReturned").html("$" + totReturned.toLocaleString());
        }
    }
}       // angelcalcView

var CurrentView = {
    topNav: "angel",
    view: null,
    restore: function() {
        var cookies = Cookies.get();
        if ("angelcalc-top" in cookies) {
            this.topNav = cookies["angelcalc-top"];
            // console.debug("chooseNav: cookie nav:" + nav);
        } else {
            this.topNav = "angel";
        }
        AngelView.restore();
        FundsView.restore();
        if (this.topNav == "angel")
            this.view = AngelView;
        else
            this.view = FundsView;
        
    }
    
}
/*
 * object used to keep track of the total performance data.
 */
function PerformanceTotalsConstructor()  {
    this.invested   =    0;
    this.returned   =    0;
    this.currVal    =    0;     // for funds, total cap account value for angel invest total current value
    this.predVal    =    0;     // predicted total return for funds or angel investments
    this.carryOnlyVal =  0;     // for funds only, the current capital account of carry only funds
    this.carryOnlyPredVal = 0;  // for funds only, the predicted return of carry only funds


    this.clear = function() {
        this.invested            = 0;
        this.returned            = 0;
        this.currVal             = 0;
        this.predVal             = 0;
        this.carryOnlyVal        = 0;
        this.carryOnlyPredVal    = 0;
    }
    this.calcIandR = function(years) {
        for (var y in years) {
            // console.debug("PerformanceTotalsConstructor: y, (I, R)" + y + ", " + angelParens(years[y].invest_amount, years[y].return_amount));
            if (Number(y) <= CurrentYear) {
                this.invested += years[y].invest_amount;
                this.returned += years[y].return_amount;
            }
        }
    };

    this.calcValue = function(type) {
        var val = 0, predval = 0;
        var shares = 0;
        var co, fund;

        if (type == "company") {
            for (var c in Companies.map) {
                co = Companies.map[c];
                if (co.status == "active") {
                    if (co.sharePrice > 0) {
                        shares = Companies.shares(co.id);
                        val += shares * co.sharePrice;
                    }
                    if ("exitSharePrice" in co && co.exitSharePrice > 0) {
                        shares = Companies.shares(co.id);
                        predval += shares * co.exitSharePrice;
                    }
                }
            }
        } else {        // fund
            for (var f in Funds.map) {
                fund = Funds.map[f];
                if (fund.status == "active") {
                        if ("capitalAccount" in fund && fund.capitalAccount > 0)
                            val += fund.capitalAccount;
                        if ("predictedReturn" in fund && fund.predictedReturn > 0)
                            predval += fund.predictedReturn;
                    }
            }
        }
        console.debug("this.calcCurrentValue (val, predval): " + angelParens(Math.round(val), Math.round(predval)));
        this.currVal = Math.round(val);
        this.predVal = Math.round(predval);

    };


    this.calcCarryValue = function() {
        var val = 0, predval=0;
        var shares = 0;
        var co, fund;


        for (var f in Funds.map) {
            fund = Funds.map[f];
            if (fund.status == "active" && !(fund.id in FundInvestments.commitMap)) {
                if ("capitalAccount" in fund && fund.capitalAccount > 0)
                    val += fund.capitalAccount;

                if ("predictedReturn" in fund && fund.predictedReturn > 0)
                    predval += fund.predictedReturn;
            }
        }

        console.debug("this.calcCarryValue (val, predval): "+ angelParens(Math.round(val), Math.round(predval)));
        this.carryOnlyVal = Math.round(val);
        this.carryOnlyPredVal = Math.round(predval);

    };
}   // performanceTotalsConstructor




var Investments;
var Companies;
var Payouts;
var MyInvestments;      // this will always be the current user's investments
var MyCompanies;        // and this will always be the current user's companies.
var MyPayouts;

var TrackerList = [];
var TrackerMap = {};
var TrackedPeople = [];
var TrackedCompanies = [];

var Funds;

var InvestorID = 0;
var RowBackgrounds = new Array('#F6F6FA', '#FdFdFA');


//var AngelTotals, AngelTotalsFiltered, FundTotalsCarry, FundTotalsInvest;

function  TotalsConstructor()  {
    this.invested        = 0;           // total invested over time
    this.returned        = 0;           // total returned over time
    this.currVal         = 0;           // for funds, total cap account value for angel  total current value
    this.predVal         = 0;           // for funds predicted total return for angel predicted total value

    this.years           = {};          // angel returns by year
    this.newCosPerYear   = {};          // number of new companies invested in per year (not yet implemented)
    this.invsPerYear     = {};          // total investments made per year
    this.filter = "all";



    this.clear = function() {
        this.invested            = 0;
        this.returned            = 0;
        this.currVal             = 0;
        this.predVal             = 0;

        for (var year in this.years) {
            var y = this.years[year];
            y.invest_amount = 0;
            y.return_amount = 0;
            this.newCosPerYear[year] = 0;
            this.invsPerYear[year] = 0;
        }
        for (year in this.filteredYears) {
            y = this.filteredYears[year];
            y.invest_amount = 0;
            y.return_amount = 0;
        }
    };
    this.reset = function() {
        this.clear();
        this.countPayouts();
        this.countInvestments();
        this.calcIandR();
        this.calcValue();
    };
    this.initialize = function() {
        for (var y=CurrentYear - 25; y < CurrentYear + 25; y++) {
            var ystr = y.toString();
            this.years[ystr] = Object.create(oneYearTotals);
            this.years[ystr].invest_amount = 0;
            this.years[ystr].return_amount = 0;
            this.newCosPerYear[ystr] = 0;
            this.invsPerYear[ystr] = 0;
        }
        this.countPayouts();
        this.countInvestments();
        this.calcIandR();
        this.calcValue();
        // console.debug("initialize: " + JSON.stringify(this.years));
    };
    this.countPayouts = function() {
        for (var i=0; i<Payouts.list.length; i++) {
            var p = Payouts.list[i];
            co = Companies.company(p.companyID);
            if (this.filter == "all" || co.status == this.filter) {
                var d = new Date(p.payDate);
                var year = d.getUTCFullYear();
                year = year.toString();

                if (p.type == "cash")
                    this.years[year].return_amount += Number(p.cash);
                else
                    this.years[year].return_amount += Math.round(Number(p.shares)* Number(p.sharePrice));
            }
        }
    };

    this.countInvestments = function() {
        // console.debug("*****here: list length: " + Investments.list.length);
        for (var i=0; i<Investments.list.length; i++) {
            var inv = Investments.list[i];
            co = Companies.company(inv.company_id);
            if (this.filter == "all" || co.status == this.filter) {
                // console.debug("*****there");
                var d = new Date(inv.inv_date);
                var year = d.getUTCFullYear();
                year = year.toString();
                // console.debug("angelreturnsbyyear: i: " + i + " || " + JSON.stringify(inv));
                if (!(year in this.years) || !(year in this.invsPerYear))
                    continue;
                this.years[year].invest_amount += Number(inv.invest_amount);
                this.invsPerYear[year]++;
            }
        }
    };


    this.calcIandR = function() {
        for (var y in this.years) {

            // console.debug("TotalsConstructor: y, (I, R)" + y + ", " + angelParens(this.years[y].invest_amount, this.years[y].return_amount));
            if (Number(y) <= CurrentYear) {
                this.invested += this.years[y].invest_amount;
                this.returned += this.years[y].return_amount;
            }
        }

        // console.debug("TotalsConstructor: DONE: filter: (I, R): " + this.filter + " : " + angelParens(this.invested, this.returned));
    };
    this.calcValue = function() {
        var val = 0, predval = 0;
        var shares = 0;
        var co;


        for (var c in Companies.map) {
            co = Companies.map[c];

            if (this.filter == "all" || co.status == this.filter) {

                if (co.status == "active") {
                    if (co.sharePrice > 0) {
                        shares = Companies.shares(co.id);
                        val += Math.round(Number(shares) * Number(co.sharePrice));
                    }
                    if ("exitSharePrice" in co && co.exitSharePrice > 0) {
                        shares = Companies.shares(co.id);
                        predval += Math.round(Number(shares) * Number(co.exitSharePrice));
                    }
                }
            }
        }

        // console.debug("this.calcCurrentValue (Companies) (val, predval): " + angelParens(Math.round(val), Math.round(predval)));
        this.currVal = Math.round(val);
        this.predVal = Math.round(predval);

    };
}

/*
 * this is the main whoopie page - perhaps someday have a home page to enter, but for now, just this
 */

function initializeWhoopie() {
     
    console.debug("initialize Whoopie!");

    /*
     * Order of Operations
     *    1) Wait for players
     *    2) Determine who deals first and how many deals up / down we will have
     *    3) Play game
     *        
     * Play Game
     *      while more hands to play {
     *          1) Deal cards
     *          2) Bid when it is your turn
     *          3) Play hand
     *          4) Score results
     *      }
     *  End Game
     */


    //Start by initalizing the library
    cards.init({table:'#whoopieTable', blackJoker:true, redJoker:true});
    //Create a new deck of cards
    deck = new cards.Deck(); 
    //cards.all contains all cards, put them all in the deck
    deck.addCards(cards.all); 
    //No animation here, just get the deck onto the table.
    deck.render({immediate:true});

    //Now lets create a couple of hands, one face down, one face up.
    hand1 = new cards.Hand({faceUp:false, x:150, y:50});
    hand2 = new cards.Hand({faceUp:false, x:450, y:50});
    hand3 = new cards.Hand({faceUp:false, x:150, y:350});
    
    yourhand = new cards.Hand({faceUp:true, x:450, y:350});
    
    //Deck has a built in method to deal to hands.
    deck.deal(1, [yourhand, hand1, hand2, hand3], 20)
    deck.x += 200;
    //deck.y -= 50;
    deck.render({immediate:true});

    // Let's turn over the top card which is the Whoopie card

    discardPile = new cards.Deck({faceUp:true});
    discardPile.x += 200;
    deck.render({callback:function() {
        discardPile.addCard(deck.topCard());
        discardPile.render();
    }});

}

function modelSummaryAction(action) {
    // console.debug("modelSummaryAction: (action, savedModelID): " + angelParens(action, SavedModelID));

    $('#modelSummaryCaptable').css("display", "none");
    if (action == "editModel") {
        if (SavedModelID == 0)
            url = "/model?sample";     // guest login - sample model page
        else
            url = "/model?editModel=" + SavedModelID;
        window.location.href = url;
    } else if (action == "deleteModel") {
        if (SavedModelID == 0)
            alert("You cannot delete this model, it's just a sample.");
        else
            deleteModel(SavedModelID, SavedModelName);
    } else if (action == "calcModel") {
        calcModelFromID(SavedModelID);
    } else
        console.debug("unknown modelSummaryAction: " + action);
}

function updateHomeCapTable(equity) {

    var postValuation = equity.preMoneyVal + equity.totalInvested;
    var vcOwnership  = equity.totalShares / equity.postFDShares;
    var commonOwnership  = (equity.preFDShares-equity.preUnallocOptions) / equity.postFDShares;
    var optOwnership  = equity.postOptions / equity.postFDShares;
    /*
     * then all the converts
     */
    var totalConvShares = 0;
    for (i=0; i < ConvertList.length; i++) {
        conv = ConvertList[i];
        totalConvShares += conv.totalShares;
    }
    var convOwnership = totalConvShares / Equity.postFDShares;

    var ownershipTotal = vcOwnership+commonOwnership+optOwnership+convOwnership;

    $('#modelName').html(SavedModelName);
    $('#postVal').html("$" + postValuation.toLocaleString());
    $('#postFDShares').html(Number(equity.postFDShares.toFixed(0)).toLocaleString());
    $('#postOptions').html(Number(equity.postOptions.toFixed(0)).toLocaleString());
    $('#postPrice').html("$" + String(Equity.price.toFixed(4)));
    $('#postVCOwnership').html((vcOwnership*100).toFixed(4) + "%");
    $('#postConvOwnership').html((convOwnership*100).toFixed(4) + "%");
    $('#postCommonOwnership').html((commonOwnership*100).toFixed(4) + "%");
    $('#postOptionsPercent').html((optOwnership*100).toFixed(2) + "%");
    $('#postOwnershipTotal').html((ownershipTotal*100).toFixed(4) + "%");

    //$('#modelSummaryCaptable').css("display", "inline");
    $("#modelSummaryCaptable").slideToggle();

}

function deleteModel(id, name) {
    // var tr = $(t).closest("tr")
    console.debug("deleteModel id: " + id);


    var modName = document.getElementById('modelToDelete');
    modName.innerHTML = name;


    $("#dialogDelete").dialog({
        resizable: false,
        height: 200,
        width: 350,
        modal: true,
        buttons: {
            Proceed: function() {
                proceed(id);
                $(this).dialog("close");
            },
            Cancel: function() {
                $(this).dialog("close");
            }
        }
    });

    function proceed(id){

        url = "model?deleteModel="+id;
        window.location.href = url;
    }

}   // deleteModel

var SavedModelID = 0;
var SavedModelName = "";
function showModelSummaryActions(element,modelID,modelName) {
    console.debug("showModelSummaryActions");
    var slider = document.getElementById("modelSummaryActionSlider");

    //console.debug("here: " + element.className);
    console.debug("modelID ,pos.top,pos.left: " + modelID + "," + $(element).position().top + "," + $(element).position().left);


    SavedModelID = modelID;
    SavedModelName = modelName;
   // slider.style.display = "inline";

    var offset = $(element).offset();

    offset.left += 268;

    $("#modelSummaryActionSlider").css(offset);
    $("#modelSummaryActionSlider").css("display","inline");



}

function showModelSummaryCaptable(element) {

    var cap = document.getElementById("modelSummaryCaptable");
    console.debug("pos.top,pos.left: " + $(element).position().top + "," + $(element).position().left);

    if (cap.style.display == "inline")
        cap.style.display = "none";
    else {
        cap.style.display = "inline";
    }

    var offset = $(element).offset();
    console.debug(offset.left + "," + offset.top);
    offset.left += 100;


    $("#modelSummaryCaptable").css(offset);

}


function updateModelTable(modNum) {

    var postVal = document.getElementById('postVal'+modNum);
    var postShares = document.getElementById('postFDShares'+modNum);
    var preShares = document.getElementById('preFDShares'+modNum);
    var postPrice = document.getElementById('postPrice'+modNum);
    //var postOptions = document.getElementById('postOptions'+modNum);
    var postOptionsPercent = document.getElementById('postOptionsPercent'+modNum);
    var postVCOwnership = document.getElementById('postVCOwnership'+modNum);
    //var postCommonOwnership = document.getElementById('postCommonOwnership'+modNum);
    var postFounderOwnership = document.getElementById('postFounderOwnership'+modNum);
    var postConvOwnership = document.getElementById('postConvOwnership'+modNum);
    var postYourOwnership = document.getElementById('postYourOwnership'+modNum);
    // var postOwnershipTotal = document.getElementById('postOwnershipTotal'+modNum);

    var ownershipTotal = 0;


    var postValuation = Equity.preMoneyVal + Equity.totalInvested;

    postVal.innerHTML       = "$" + postValuation.toLocaleString();
    // postOptions.innerHTML   = Number(Equity.postOptions.toFixed(0)).toLocaleString();
    postShares.innerHTML    = Number(Equity.postFDShares.toFixed(0)).toLocaleString();
    preShares.innerHTML    = Number(Equity.preFDShares.toFixed(0)).toLocaleString();
    var newPrice = (postValuation/Equity.postFDShares);
    postPrice.innerHTML     = "$" + String(Equity.price.toFixed(4));

    var optOwnership  = Equity.postOptions / Equity.postFDShares;
    postOptionsPercent.innerHTML = (optOwnership*100).toFixed(2) + "%";

    ownershipTotal += optOwnership;

    var vcOwnership  = Equity.totalShares / Equity.postFDShares;
    postVCOwnership.innerHTML = (vcOwnership*100).toFixed(4) + "%";

    ownershipTotal += vcOwnership;

   /*var commonOwnership  = (Equity.preFDShares - Equity.preUnallocOptions) / Equity.postFDShares;
    postCommonOwnership.innerHTML = (commonOwnership*100).toFixed(4) + "%";*/

    var founderOwnership  = Equity.founderTotalEquity / Equity.postFDShares;
    postFounderOwnership.innerHTML = (founderOwnership*100).toFixed(4) + "%";

    ownershipTotal += founderOwnership;


    var convOwnership = Equity.convertShares / Equity.postFDShares;
    postConvOwnership.innerHTML = (convOwnership*100).toFixed(4) + "%";

    var yourOwnership = Equity.totalYourShares / Equity.postFDShares;
    postYourOwnership.innerHTML = (yourOwnership*100).toFixed(4) + "%";

}

function updateModelDifferenceTable(e1, e2) {


    var postVal = document.getElementById('postValD');
    var postShares = document.getElementById('postFDSharesD');
    var preShares = document.getElementById('preFDSharesD');
    var postPrice = document.getElementById('postPriceD');
    //var postOptions = document.getElementById('postOptionsD');
    var postOptionsPercent = document.getElementById('postOptionsPercentD');
    var postVCOwnership = document.getElementById('postVCOwnershipD');
    // var postCommonOwnership = document.getElementById('postCommonOwnershipD');
    var postFounderOwnership = document.getElementById('postFounderOwnershipD');
    var postConvOwnership = document.getElementById('postConvOwnershipD');
    var postYourOwnership = document.getElementById('postYourOwnershipD');
    // var postOwnershipTotal = document.getElementById('postOwnershipTotal'+modNum);

    var ownershipTotal = 0;




    var postValuation1 = e1.preMoneyVal + e1.totalInvested;
    var postValuation2 = e2.preMoneyVal + e2.totalInvested;

    postVal.innerHTML       = "$" + Number(postValuation1 - postValuation2).toLocaleString();
    // postOptions.innerHTML   = Number(Equity.postOptions.toFixed(0)).toLocaleString();
    postShares.innerHTML = Number(e1.postFDShares.toFixed(0) - e2.postFDShares.toFixed(0)).toLocaleString();
    preShares.innerHTML = Number(e1.preFDShares.toFixed(0) - e2.preFDShares.toFixed(0)).toLocaleString();

    postPrice.innerHTML     = "$" + String((e1.price-e2.price).toFixed(4));

    var optOwnership1  = e1.postOptions / e1.postFDShares;
    var optOwnership2  = e2.postOptions / e2.postFDShares;

    postOptionsPercent.innerHTML = ((optOwnership1-optOwnership2)*100).toFixed(2) + "%";

    var vcOwnership1  = e1.totalShares / e1.postFDShares;
    var vcOwnership2  = e2.totalShares / e2.postFDShares;
    postVCOwnership.innerHTML = ((vcOwnership1-vcOwnership2)*100).toFixed(4) + "%";

    /*var commonOwnership1  = (e1.preFDShares-e1.preUnallocOptions) / e1.postFDShares;
    var commonOwnership2  = (e2.preFDShares-e2.preUnallocOptions)/ e2.postFDShares;
    postCommonOwnership.innerHTML = ((commonOwnership1-commonOwnership2)*100).toFixed(4) + "%";*/

    var founderOwnership1  = (e1.founderTotalEquity) / e1.postFDShares;
    var founderOwnership2  = (e2.founderTotalEquity)/ e2.postFDShares;
    postFounderOwnership.innerHTML = ((founderOwnership1-founderOwnership2)*100).toFixed(4) + "%";

    var convertOwnership1  = e1.convertShares / e1.postFDShares;
    var convertOwnership2  = e2.convertShares / e2.postFDShares;
    postConvOwnership.innerHTML = ((convertOwnership1-convertOwnership2)*100).toFixed(4) + "%";

    var yourOwnership1  = e1.totalYourShares / e1.postFDShares;
    var yourOwnership2  = e2.totalYourShares / e2.postFDShares;
    postYourOwnership.innerHTML = ((yourOwnership1-yourOwnership2)*100).toFixed(4) + "%";

}



function cleanUpOutput() {

    var outputDiv = document.getElementById('outputDiv');

    outputDiv.style.display = "none";

    /*****************************************************************
     * Start by cleaning up -- remove saved converts and rows in output
     */
    var len = ConvertList.length;

    var eqTable = document.getElementById("equationTable");


    $("#convertOwnershipTable tbody tr").remove();
    $("#ownershipTable tbody tr").remove();
    $("#founderOwnershipTable tbody tr").remove();
    $("#otherCommonOwnershipTable tbody tr").remove();


    for (i=0; i < len; i++) {
        eqTable.deleteRow(-1);  // converts are at the end
    }

    // now kill the converts
    for (i=0; i < len; i++)
        ConvertList.pop();



    /*****************************************************************/

}
function createCommonLists(modNum) {
    var founderCount = Number($("#founderCount"+modNum).val());
    var otherCommonCount = Number($("#otherCommonCount"+modNum).val());

    for (var i=0; i < founderCount; i++) {
        Equity.founderEquity[i] = Number($("#founder" + i + "Shares" + modNum).val());
        Equity.founderNames[i] = $("#founder" + i + "Name" + modNum).val();
        Equity.founderTotalEquity += Equity.founderEquity[i];
    }
    Equity.numFounders = founderCount;
    for (var i=0; i < otherCommonCount; i++) {
        Equity.commonEquity[i] = Number($("#commonHolder" + i + "Shares" + modNum).val());
        Equity.commonNames[i] = $("#commonHolder" + i + "Name" + modNum).val();
        Equity.commonTotalEquity += Equity.commonEquity[i];
    }
    Equity.numOtherCommon = otherCommonCount;
    console.debug("createCommonLists: (Count, TotalEquity): " + angelParens(Equity.numFounders, Equity.founderTotalEquity));
}

function createConvertList(modNum) {
    var convertCount = Number(document.getElementById("convertCount"+modNum).value);

    for (var i=0; i < convertCount; i++) {
        conv = Object.create(Convert);

        conv.yourInvestment  = Number(document.getElementById('CONV'+i+'convAmount'+modNum).value);      // amount you invested
        conv.totalInvested   = Number(document.getElementById('CONV'+i+'convTotal'+modNum).value);       // total amount converts invested
        conv.discount    = Number(document.getElementById('CONV'+i+'discount'+modNum).value);
        conv.cap         = Number(document.getElementById('CONV'+i+'cap'+modNum).value);
        conv.type         = document.getElementById('CONV'+i+'type'+modNum).value;
        if (conv.type == "Custom") {
            conv.custom = document.getElementById('CONV'+i+'custom'+modNum).value;
            conv.preexist = document.getElementById('CONV'+i+'preexist'+modNum).value;
        } else if (conv.type == "YCVC SAFE")
            conv.preexist = document.getElementById('CONV'+i+'preexist'+modNum).value;
        else if (conv.type == "Manual") {
            conv.totalShares = Number(document.getElementById('CONV'+i+'shares'+modNum).value);
            conv.price = Precision.round(conv.totalInvested / conv.totalShares);
        }

        conv.yourInvestment  = Number($('#CONV'+i+'convAmount'+modNum).val());
        conv.totalInvested   = Number($('#CONV'+i+'convTotal'+modNum).val());
        conv.discount    = Number($('#CONV'+i+'discount'+modNum).val());
        conv.cap         = Number($('#CONV'+i+'cap'+modNum).val());
        conv.type         = document.getElementById('CONV'+i+'type'+modNum).value;
        if (conv.type == "Custom") {
            conv.custom = document.getElementById('CONV'+i+'custom'+modNum).value;
            conv.preexist = document.getElementById('CONV'+i+'preexist'+modNum).value;
        } else if (conv.type == "YCVC SAFE")
            conv.preexist = document.getElementById('CONV'+i+'preexist'+modNum).value;
        else if (conv.type == "Post SAFE")
            conv.custom = document.getElementById('CONV'+i+'custom'+modNum).value;
        else if (conv.type == "YC POST")
            conv.custom = document.getElementById('CONV'+i+'custom'+modNum).value;
        else if (conv.type == "Manual") {
            conv.totalShares = Number($('#CONV'+i+'shares'+modNum).val());
            conv.price = Precision.round(conv.totalInvested / conv.totalShares);
        }

        console.debug("createConvertList: created conv of type: " + conv.type + " modNum: " + modNum);
        ConvertList[i] = conv;
    }
}

/**************************************
 *
 * @param oSharesPost
 * @param totalFDpreShares
 * @param existingOptions
 * @param discountPrice
 * @returns {number}
 */
function convertConverts(oSharesPost, totalFDpreShares, existingOptions, equityPrice) {

    var totalShares = 0;
    var convertPrice = 0;
    var shares = 0;
    var id = "";
    var discount;  // actual percent
    var i = 0;

    for (i=0; i < ConvertList.length; i++) {
        conv = ConvertList[i];
        if (conv.totalInvested == 0)
            continue;       /* empty convertible */

        /*
         * calculate price based on each type of convertible
         */
        if (conv.type == "SAFE" || conv.type == "YCII SAFE") {
            if (conv.cap != 0) {
                if (conv.discount == 0)
                    conv = ycStdSafeCap(conv, oSharesPost, totalFDpreShares, existingOptions, equityPrice, Equity.preMoneyVal);
                else
                    conv = ycStdSafeCapDiscount(conv, oSharesPost, totalFDpreShares, existingOptions, equityPrice);
            } else
                    conv = ycStdSafeDiscount(conv, oSharesPost, totalFDpreShares, existingOptions, equityPrice, Equity.preMoneyVal);

        } else {
            if (conv.type == "Note") {
                // I've got nothin' right now if it's not a simple one with a cap. I think the others might be the same as a SAFE!
                if (conv.cap != 0) {
                    if (conv.discount == 0)
                        conv = ycStdNoteCap(conv, oSharesPost, totalFDpreShares, existingOptions, equityPrice);
                    else
                        conv = ycStdSafeCapDiscount(conv, oSharesPost, totalFDpreShares, existingOptions, equityPrice);
                } else
                        conv = ycStdSafeDiscount(conv, oSharesPost, totalFDpreShares, existingOptions, equityPrice, Equity.preMoneyVal);

            } else {
                if (conv.type == "YCVC SAFE")
                    conv = ycvcSafe(conv, i, oSharesPost, totalFDpreShares, existingOptions, equityPrice);
                else if (conv.type == "YC POST")
                    conv = ycPost(conv, i, oSharesPost, totalFDpreShares, existingOptions, equityPrice);
                else if (conv.type == "Post SAFE") {
                    /*if (conv.discount == 0)
                        conv = postMoneySafeCap(conv, i, oSharesPost, totalFDpreShares, existingOptions, equityPrice, Equity.preMoneyVal);
                    else
                        conv = postMoneySafeCapDiscount(conv, i, oSharesPost, totalFDpreShares, existingOptions, equityPrice);*/

                    conv = postMoneySafeCapDiscount(conv, i, oSharesPost, totalFDpreShares, existingOptions, equityPrice, Equity.preMoneyVal);

                } else if (conv.type == "Custom")
                    conv = customConvert(conv, i, oSharesPost, totalFDpreShares, existingOptions, equityPrice);
                else if (conv.type == "Manual") {
                    conv.equationNumerator = "N/A";
                    conv.equationDenominator = "";

                } else
                    console.debug("help!!!!")
            }
        }

        totalShares += conv.totalShares;
        // console.log("***convertConverts: convTotal=" + conv.totalInvested + " discount= " + conv.discount + " cap = " + conv.cap + " convertPrice=" + conv.price +  " shares=" + conv.totalShares);

    }

    return(totalShares);

}


/***************************************************************
 * Calculates the per-share price for a YC Standard safe converting with a cap and a discount
 * Price = lower of Safe Price and Discount Price
 *
 * Safe Price = Safe Cap / Fully Diluted Capitalization plus new option shares  (does not include other converts)
 * Discount Price = (1-discount)* Equity Price
 *
 * @param conv
 * @param oSharesPost
 * @param totalFDpreShares
 * @param existingOptions
 * @param equityPrice
 * @returns {*}
 */
function ycStdSafeCapDiscount(conv, oSharesPost, totalFDpreShares, existingOptions, equityPrice) {

    var safePrice = 0;
    var discountPrice = 0;

    discount = conv.discount / 100;

    if (oSharesPost == 0)
        safePrice = Precision.round(conv.cap / totalFDpreShares);    /* no new options and old ones included in preFD */
    else
        safePrice = Precision.round(conv.cap / (oSharesPost + totalFDpreShares - existingOptions));

    discountPrice = Precision.round((1-discount) * equityPrice);

    if (discountPrice == 0 || safePrice < discountPrice) {
        conv.price = safePrice;

        conv.equationNumerator = Number(conv.cap.toFixed(0)).toLocaleString() + " (cap)";

        if (oSharesPost == 0) {
            /*
             * There are no new options, so existing options are included in the pre-fd shares
             */
            conv.equationDenominator = Number(totalFDpreShares.toFixed(0)).toLocaleString()  + " (FD Pre Shares)";
        } else
            conv.equationDenominator = Number(oSharesPost.toFixed(0)).toLocaleString() + " (options) + " +
                Number(totalFDpreShares.toFixed(0)).toLocaleString()  + " (FD Pre Shares) - " +
                Number(existingOptions.toFixed(0)).toLocaleString()  + " (existing options)";
    } else {
        conv.price = discountPrice;

        conv.equationNumerator = equityPrice.toFixed(4) + " (Equity price) * " + (1-discount) * 100 + "% (1-discount)";
        conv.equationDenominator = "";

    }

    conv.totalShares = Precision.round(conv.totalInvested / conv.price);
    conv.yourShares = Precision.round(conv.yourInvestment / conv.price);

    // console.debug("ycStdSafeCapDiscount: "  + "conv.cap: " + conv.cap.toLocaleString() + " conv.discount: " + conv.discount + " discountPrice: " + discountPrice.toFixed(4) + " conv.price: " + conv.price.toFixed(4));

    return(conv);
}

/***************************************************************
 * Calculates the per-share price for a YC Standard safe converting with a cap and no discount
 * Price = Equity Price if the pre-money valuation is less than or equal to the safe cap and otherwise
 * Price = Safe Cap / Fully Diluted Capitalization plus new option shares  (does not include other converts)
 *
 * @param conv
 * @param oSharesPost
 * @param totalFDpreShares
 * @param existingOptions
 * @param equityPrice
 * @oaram preMoneyVal
 * @returns {*}
 */
function ycStdSafeCap(conv, oSharesPost, totalFDpreShares, existingOptions, equityPrice, preMoneyVal) {

    var safePrice = 0;

    if (oSharesPost == 0)
        safePrice = Precision.round(conv.cap / totalFDpreShares);    /* no new options and old ones included in preFD */
    else
        safePrice = Precision.round(conv.cap / (oSharesPost + totalFDpreShares - existingOptions));

    if (equityPrice == 0 || conv.cap < preMoneyVal) {
        conv.price = safePrice;

        conv.equationNumerator = Number(conv.cap.toFixed(0)).toLocaleString() + " (cap)";

        if (oSharesPost == 0) {
            /*
             * There are no new options, so existing options are included in the pre-fd shares
             */
            conv.equationDenominator = Number(totalFDpreShares.toFixed(0)).toLocaleString()  + " (FD Pre Shares)";
        } else
             conv.equationDenominator = Number(oSharesPost.toFixed(0)).toLocaleString() + " (options) + " +
                Number(totalFDpreShares.toFixed(0)).toLocaleString()  + " (FD Pre Shares) - " +
                Number(existingOptions.toFixed(0)).toLocaleString()  + " (existing options)";
    } else {
        conv.price = equityPrice;

        conv.equationNumerator = equityPrice.toFixed(4) + " (Equity price)";
        conv.equationDenominator = "";

    }

    conv.totalShares = Precision.round(conv.totalInvested / conv.price);
    conv.yourShares = Precision.round(conv.yourInvestment / conv.price);

    console.debug("ycStdSafeCap: " + "conv.cap: " + conv.cap.toLocaleString() + " conv.price: " + conv.price.toFixed(4)+ " equityPrice: " + equityPrice.toFixed(4));

    return(conv);
}

/***************************************************************
 * Calculates the per-share price for a YC Standard safe converting with a discount only
 * Price = Discount Price = (1-discount)* Equity Price
 *
 * @param conv
 * @param oSharesPost
 * @param totalFDpreShares
 * @param existingOptions
 * @param equityPrice
 * @oaram preMoneyVal
 * @returns {*}
 */
function ycStdSafeDiscount(conv, oSharesPost, totalFDpreShares, existingOptions, equityPrice, preMoneyVal) {

    var discountPrice = 0;

    discount = conv.discount / 100;

    discountPrice = Precision.round((1-discount) * equityPrice);

    if (discountPrice == 0) {
        /*
         * this is a special case where we are trying to approximate the equityPrice. We will never really use
         * this price so I am not bothering with the equation. This function is only called for real with an
         * actual equityPrice and therefore discountPrice. (see calcCapTable)
         */
        conv.price = Precision.round(preMoneyVal / (oSharesPost + totalFDpreShares - existingOptions));

    } else {
        conv.price = discountPrice;

        conv.equationNumerator = equityPrice.toFixed(4) + " (Equity price) * " + (1-discount) * 100 + "% (1-discount)";
        conv.equationDenominator = "";

    }

    conv.totalShares = Precision.round(conv.totalInvested / conv.price);
    conv.yourShares = Precision.round(conv.yourInvestment / conv.price);

    console.debug("ycStdSafeDiscount: conv.discount: " + conv.discount + " discountPrice: " + discountPrice.toFixed(4) + " conv.price: " + conv.price.toFixed(4));

    return(conv);
}

/***************************************************************
 * Calculates the per-share price for a YC Standard note converting with a cap and no discount
 * Price is the lower of the Equity Price and the Target Price where
 * Target Price = Note Cap / Fully Diluted Capitalization plus new option shares  (does not include other converts)
 *
 * @param conv
 * @param oSharesPost
 * @param totalFDpreShares
 * @param existingOptions
 * @param equityPrice
 * @oaram preMoneyVal
 * @returns {*}
 */
function ycStdNoteCap(conv, oSharesPost, totalFDpreShares, existingOptions, equityPrice) {

    var targetPrice = 0;


    targetPrice = Precision.round(conv.cap / (oSharesPost + totalFDpreShares - existingOptions));

    if (equityPrice == 0 || targetPrice < equityPrice) {
        conv.price = targetPrice;

        conv.equationNumerator = Number(conv.cap.toFixed(0)).toLocaleString() + " (cap)";

        conv.equationDenominator = Number(oSharesPost.toFixed(0)).toLocaleString() + " (options) + " +
            Number(totalFDpreShares.toFixed(0)).toLocaleString()  + " (FD Pre Shares) - " +
            Number(existingOptions.toFixed(0)).toLocaleString()  + " (existing options)";
    } else {
        conv.price = equityPrice;

        conv.equationNumerator = equityPrice.toFixed(4) + " (Equity price)";
        conv.equationDenominator = "";

    }

    conv.totalShares = Precision.round(conv.totalInvested / conv.price);
    conv.yourShares = Precision.round(conv.yourInvestment / conv.price);

    console.debug("ycStdNoteCap: " + "conv.cap: " + conv.cap.toLocaleString() + " conv.price: " + conv.price.toFixed(4)+ " equityPrice: " + equityPrice.toFixed(4));

    return(conv);
}

/***************************************************************
 * Calculates the per-share price for a YCVC SAFE
 * Price is the Safe Price which is:
 * Safe Price = Note Cap (usually 10mm) / Fully Diluted Capitalization plus new option shares  plus all existing converts - including THIS ONE
 *    we currently only include this SAFE, which means our price will be a bit high in those cases until we write more code!
 *
 *    the formula we use is: cap - investment / (denominator above): (investment = i, price = p, s = safe shares denom = d + s )
 *
 *         s = i / p  and  p = cap / d + s  --> p =  p * (d + s) = cap  --> pd + ps = cap --> pd + i = cap --> p = (cap - i) / d
 *
 * @param conv
 * @param oSharesPost
 * @param totalFDpreShares
 * @param existingOptions
 * @param equityPrice
 * @oaram preMoneyVal
 * @returns {*}
 */
function ycvcSafe(conv, myIndex, oSharesPost, totalFDpreShares, existingOptions, equityPrice) {

    var safePrice = 0;

    var convsToAdd = conv.preexist;
    var i = 0;

    /*
     * a YCVC safe's price calculation should include all preexisting SAFE's in its denominator. These are included
     * in the preexist element.
     */
    var convSharesToAddDenom = "";
    var convSharesToAdd = 0;

    c = preexistCount(convsToAdd, myIndex);
    convSharesToAdd = c.totalShares;
    convSharesToAddDenom = c.equationDenominator;

    safePrice = Precision.round((conv.cap - conv.totalInvested)/ (oSharesPost + totalFDpreShares - existingOptions + convSharesToAdd));
    conv.price = safePrice;

    conv.totalShares = Precision.round(conv.totalInvested / conv.price);

    conv.equationNumerator = Number(conv.cap.toFixed(0)).toLocaleString() + " (cap)";

    conv.equationDenominator = Number(oSharesPost.toFixed(0)).toLocaleString() + " (options) + " +
            Number(totalFDpreShares.toFixed(0)).toLocaleString()  + " (FD Pre Shares) - " +
            Number(existingOptions.toFixed(0)).toLocaleString()  + " (existing options) +" +
            Number(conv.totalShares.toFixed(0)).toLocaleString()  + " (this safe's shares)" + convSharesToAddDenom;

    // console.debug("ycvcSafe: " + "conv.cap: " + conv.cap.toLocaleString() + " conv.price: " + conv.price.toFixed(4));

    return(conv);
}


/***************************************************************
 * Calculates the per-share price for a YC POST Money Safe
 * Price is the Safe Price which is:
 * Safe Price = Note Cap (usually 10mm) / Fully Diluted Capitalization plus new option shares  plus all existing converts - including THIS ONE
 *    we currently only include this SAFE, which means our price will be a bit high in those cases until we write more code!
 *
 *    the formula we use is: cap - investment / (denominator above): (investment = i, price = p, s = safe shares denom = d + s )
 *
 *         s = i / p  and  p = cap / d + s  --> p =  p * (d + s) = cap  --> pd + ps = cap --> pd + i = cap --> p = (cap - i) / d
 *
 * @param conv
 * @param oSharesPost
 * @param totalFDpreShares
 * @param existingOptions
 * @param equityPrice
 * @oaram preMoneyVal
 * @returns {*}
 */
function ycPost(conv, myIndex, oSharesPost, totalFDpreShares, existingOptions, equityPrice) {

    conv.cap = 150000/.07;
    conv.totalInvested = 150000;

    c = calcPostMoneySafePrice(conv, myIndex, oSharesPost, totalFDpreShares, existingOptions);

    conv.price = c.price;
    conv.equationNumerator = c.equationNumerator;
    conv.equationDenominator = c.equationDenominator;
    conv.totalShares = Precision.round(conv.totalInvested / conv.price);
    conv.yourShares = Precision.round(conv.yourInvestment / conv.price);
    console.debug("ycPost: " + "conv.cap: " + conv.cap.toLocaleString() + " conv.price: " + conv.price.toFixed(4)
    + " conv.custom: " + conv.custom);


    return(conv);
}  // ycpost


/*************************************************************
 * preexistCount calculates the denominator and share count of preexisting converts to include. these are returned in
 * a somewhat hacky way in a conv structure (sorry, but this was easist) in totalShares and denom. elements.
 *
 * @param list  -- this is the comma separated list of converts to include - we will ignore any that don't make sense
 * @param myIndex -- index of myself - ignore!
 *
 */

function preexistCount(list, myIndex) {

    var conv = Object.create(Convert);
    var addList = list.split(",");
    var convIndex;

    var convSharesToAddDenom = "";
    var convSharesToAdd = 0;
    if (addList[0] != "") {                 // there's one empty entry when the field is empty
        for (i=0; i < addList.length; i++) {
            /*
             * error check addList[i] to make sure it is legit - just ignore otherwise
             */
            convIndex = addList[i];
            if ($.isNumeric(convIndex) && convIndex >= 0 && convIndex < ConvertList.length && convIndex != myIndex) {
                c = ConvertList[convIndex];
                convSharesToAddDenom += " + " + Number(c.totalShares.toFixed(0)).toLocaleString() + " (Convert " + addList[i] + ")";
                convSharesToAdd += c.totalShares;
            }
        }
    }
    conv.totalShares = convSharesToAdd;
    conv.equationDenominator = convSharesToAddDenom;

    return(conv);
}
/***************************************************************
 * Calculates the per-share price for a what is called a Post Money SAFE
 * but really means post all other seed money and includes all of that money
 * in the denominator of the price calculation (including itself!)
 *
 * @param conv
 * @param myIndex
 * @param oSharesPost
 * @param totalFDpreShares
 * @param existingOptions
 * @param equityPrice
 * @param preMoneyVal
 */
function postMoneySafeCap(conv, myIndex, oSharesPost, totalFDpreShares, existingOptions, equityPrice, preMoneyVal) {

    if (conv.cap == 0)      // nothing to do, convert is not valid
        conv.cap = 10000000000; // make it effectively uncapped - hoping this works! :)
    if (equityPrice == 0 || conv.cap <  preMoneyVal) {
        var convSharesToAdd = 0;
        var denominator = "";

        var preexistConvs = "";

        for (i=0; i < ConvertList.length; i++) {
            if (i == myIndex)   // our shares aren't calc'd yet, don't try to include them!
                continue;
            c = ConvertList[i];
            convSharesToAdd += c.totalShares;
            denominator += " + " + Number(c.totalShares.toFixed(0)).toLocaleString() + " (Convert " + i + ")";
            preexistConvs += i + ",";

        }
        var cd = customBuildDenom(conv.custom, myIndex, preexistConvs, oSharesPost, totalFDpreShares, existingOptions);
        /* remember that the denom includes our shares, but rather than be circular, we use algebra and subtract the amount invested */
        conv.price = Precision.round((conv.cap - conv.totalInvested)/ (oSharesPost + totalFDpreShares - existingOptions + convSharesToAdd));
        conv.totalShares = Precision.round(conv.totalInvested / conv.price);
        conv.yourShares = Precision.round(conv.yourInvestment / conv.price);

        conv.equationNumerator = Number(conv.cap.toFixed(0)).toLocaleString() + " (cap)";

        conv.equationDenominator =
            Number(oSharesPost.toFixed(0)).toLocaleString() + " (options) + " +
            Number(totalFDpreShares.toFixed(0)).toLocaleString()  + " (FD Pre Shares) - " +
            Number(existingOptions.toFixed(0)).toLocaleString()  + " (existing options) + " +
            Number(conv.totalShares.toFixed(0)).toLocaleString()  + " (this safe's shares)" +  denominator;
        conv.equationDenominator = cd.equationDenominator;  // if customBuildDenom actually works!

    } else {
        conv.price = equityPrice;
        conv.totalShares = Precision.round(conv.totalInvested / conv.price);
        conv.yourShares = Precision.round(conv.yourInvestment / conv.price);

        conv.equationNumerator = equityPrice.toFixed(4) + " (Equity price)";
        conv.equationDenominator = "";

    }

    console.debug("postMoneySafeCap: " + "conv.cap: " + conv.cap.toLocaleString() + " conv.price: " + conv.price.toFixed(4));

    return(conv);

}
/******************************************************************
 *
 * @param conv
 * @param myIndex
 * @param oSharesPost
 * @param totalFDpreShares
 * @param existingOptions
 * @param equityPrice
 */
function postMoneySafeCapDiscount(conv, myIndex, oSharesPost, totalFDpreShares, existingOptions, equityPrice) {

    var discount = conv.discount / 100;
    var discountPrice = Precision.round((1-discount) * equityPrice);


    if (conv.cap == 0)
        conv.cap = 11000000000; // make it effectively uncapped - hoping this works! :)

    c = calcPostMoneySafePrice(conv, myIndex, oSharesPost, totalFDpreShares, existingOptions);

    if (discountPrice == 0 || c.price < discountPrice) {
        conv.price = c.price;

        conv.equationNumerator = c.equationNumerator;
        conv.equationDenominator = c.equationDenominator;
        conv.totalShares = Precision.round(conv.totalInvested / conv.price);
        conv.yourShares = Precision.round(conv.yourInvestment / conv.price);


    } else {
        conv.price = discountPrice;
        conv.totalShares = Precision.round(conv.totalInvested / conv.price);
        conv.yourShares = Precision.round(conv.yourInvestment / conv.price);

        conv.equationNumerator = equityPrice.toFixed(4) + " (Equity price) * " + (1-discount) * 100 + "% (1-discount)";
        conv.equationDenominator = "";

    }


    // console.debug("postMoneySafeCapDiscount: "  + "conv.cap: " + conv.cap.toLocaleString() + " conv.discount: " + conv.discount + " discountPrice: " + discountPrice.toFixed(4) + " conv.price: " + conv.price.toFixed(4) + " conv.custom: " + conv.custom);

    return(conv);


}

function calcPostMoneySafePrice(origConv, myIndex, oSharesPost, totalFDpreShares, existingOptions) {

    var conv = Object.create(Convert);
    var convSharesToAdd = 0;
    var denominator = "";
    var newOptions = 0;

    var includeUnissued = (origConv.custom.search("customDenomUnissued") != -1);
    var includeAdditional = (origConv.custom.search("customDenomAdditional") != -1);

    // console.debug("unissued / additional: " + includeUnissued + " / " + includeAdditional);

    if (includeAdditional)
        newOptions = oSharesPost - existingOptions;

    if (!includeUnissued)
        newOptions = -1*Number(existingOptions);    // subtract these from Fully diluted (they're included already)

    // console.debug("osharespost,existingoptions: " + oSharesPost +","+existingOptions);


    for (i=0; i < ConvertList.length; i++) {
        if (i == myIndex)   // our shares aren't calc'd yet, don't try to include them!
            continue;
        c = ConvertList[i];
        convSharesToAdd += c.totalShares;
        denominator += " + " + Number(c.totalShares.toFixed(0)).toLocaleString() + " (Convert " + i + ")";
    }
    /* remember that the denom includes our shares, but rather than be circular, we use algebra and subtract the amount invested */
    /*console.debug("cap,ti,newopt,totalfdpre,convshares: " + origConv.cap +","+origConv.totalInvested+","
                    +newOptions+","+totalFDpreShares+","+convSharesToAdd);*/
    conv.price = Precision.round((origConv.cap - origConv.totalInvested)/ ( newOptions + totalFDpreShares + convSharesToAdd));

    conv.totalShares = Precision.round(origConv.totalInvested / conv.price);

    conv.equationNumerator = Number(origConv.cap.toFixed(0)).toLocaleString() + " (cap)";

    if (includeAdditional)
        conv.equationDenominator = postOptionsDenom(oSharesPost, existingOptions);
    else
        conv.equationDenominator = "";

    conv.equationDenominator +=
        Number(totalFDpreShares.toFixed(0)).toLocaleString()  + " (FD Pre Shares) + " +
        Number(conv.totalShares.toFixed(0)).toLocaleString()  + " (this safe's shares)" + denominator;

    if (includeUnissued)
        conv.equationDenominator += " - 0 (unissued options included in FD shares)";
    else
        conv.equationDenominator += " - " + Number(existingOptions.toFixed(0)).toLocaleString() + " (existing unissued options included in FD) " ;

    return(conv);

}

function postOptionsDenom(sharesPost, existing) {
    var returnString = "";

    if (sharesPost >= existing) {
        /* the most commonn / normal case */
        returnString += Number(sharesPost.toFixed(0)).toLocaleString() + " (total new options) ";
        if (existing > 0)
            returnString += " - " + Number(existing.toFixed(0)).toLocaleString() + " (existing unissued options) ";
    } else {
        returnString += " (there are no new options to include) ";
    }


}

/***************************************************************
 * Calculates the per-share price for a true Post Money SAFE
 * This is not actually used because I think noone really does post money converts like this
 * (where post money is post equity money too).
 * Price is the Safe Price which is:
 * @returns {*}
 */
function postMoneySafe(conv) {


    postVal = Equity.totalInvested+Equity.preMoneyVal;

    /*
     * if the post money cap is less than the actual post valuation, use the cap to calc. price. Otherwise use
     * the actual post.
     */
    if (conv.cap < postVal) {
        conv.price = conv.cap / Equity.postFDShares;
        conv.equationNumerator = Number(conv.cap.toFixed(0)).toLocaleString() + " (post money cap)";
        conv.equationDenominator = Number(Equity.postFDShares.toFixed(0)).toLocaleString() + " (FD Shares)";
    } else {
        conv.price = postVal / Equity.postFDShares;
        conv.equationNumerator = Number(postVal.toFixed(0)).toLocaleString() + " (post money val)";
        conv.equationDenominator = Number(Equity.postFDShares.toFixed(0)).toLocaleString() + " (FD Shares)";

    }
    conv.totalShares = Precision.round(conv.totalInvested / conv.price);
    conv.yourShares  = Precision.round(conv.yourInvestment / conv.price);

    console.debug("postMoneySafe: " + "conv.cap: " + conv.cap.toLocaleString() + " postFDShares: " + Equity.postFDShares.toLocaleString());

    return(conv);
}



/***************************************************************
 * Calculates the per-share price for a custom convert converting with a cap and a discount
 * Price = lower of Safe Price and Discount Price
 *
 * Safe Price = Safe Cap / Fully Diluted Capitalization plus new option shares  (does not include other converts)
 * Discount Price = (1-discount)* Equity Price
 *
 * @param conv
 * @param oSharesPost
 * @param totalFDpreShares
 * @param existingOptions
 * @param equityPrice
 * @returns {*}
 */

function customConvert(conv, index, oSharesPost, totalFDpreShares, existingOptions, equityPrice) {

    var convPrice = 0;
    var discountPrice = 0;

    var preexistConvs = conv.preexist;

    // var dict = parseQueryStringToDictionary(conv.custom);

    var testVal;
    var testPrice;
    var discount = conv.discount / 100;

    discountPrice = Precision.round((1-discount) * equityPrice);

    // var compareValue = (dict["customCompare"] == "capAndVal");
    var compareValue = (conv.custom.search("capAndVal") != -1);
    /*
     * if we need to include ourselves, we rely on algebra to avoid a goal seek here (i.e. we don't try to
     * precalculate our number of shares. See the comment on the ycvcsafe for the equations.
     */
    // var includeMyself = ("customDenomThis" in dict);
    var includeMyself = (conv.custom.search("customDenomThis") != -1);

    /*
     * discount only convert. This is no different than any other convert, of course.
     */
    if (conv.cap == 0) {
        if (conv.discount != 0) {
            conv.price = discountPrice;

            conv.equationNumerator = equityPrice.toFixed(4) + " (Equity price) * " + (1-discount) * 100 + "% (1-discount)";
            conv.equationDenominator = "";

        }

    } else {
        c = customBuildDenom(conv.custom, index, preexistConvs, oSharesPost, totalFDpreShares, existingOptions);       /* determine total shares in this converts denom */
        /* there is a cap - and there may also be a discount */
        if (compareValue) {
            if (conv.cap < Equity.preMoneyVal) {
                if (includeMyself)
                    conv.price = Precision.round((conv.cap-conv.totalInvested) / c.totalShares);
                else
                    conv.price = Precision.round(conv.cap / c.totalShares);

                conv.equationNumerator = Number(conv.cap.toFixed(0)).toLocaleString();
                conv.equationDenominator = c.equationDenominator;

            } else {
                if (includeMyself)
                    conv.price = Precision.round((Equity.preMoneyVal-conv.totalInvested) / c.totalShares);
                else
                    conv.price = Precision.round(Equity.preMoneyVal / c.totalShares);

                conv.equationNumerator = Number(Equity.preMoneyVal.toFixed(0)).toLocaleString();
                conv.equationDenominator = c.equationDenominator;
            }

        } else {
            /* comparision will be calculated convert price vs. discounted price */
            if (includeMyself)
                convPrice = Precision.round((conv.cap - conv.totalInvested) / c.totalShares);
            else
                convPrice = Precision.round(conv.cap / c.totalShares);

            if (convPrice < discountPrice) {
                conv.price = convPrice;
                conv.equationNumerator = Number(conv.cap.toFixed(0)).toLocaleString();
                conv.equationDenominator = c.equationDenominator;
            } else {
                /* discount price to be used */
                conv.price = discountPrice;
                conv.equationNumerator = equityPrice.toFixed(4) + " (Equity price) * " + (1-discount) * 100 + "% (1-discount)";
                conv.equationDenominator = "";
            }
        }

    }

    conv.totalShares = Precision.round(conv.totalInvested / conv.price);
    conv.yourShares = Precision.round(conv.yourInvestment / conv.price);

    if (includeMyself && conv.equationDenominator != "")
        conv.equationDenominator += " + " + Number(conv.totalShares.toFixed(0)).toLocaleString()  + " (this convert's shares)";

    console.debug("customConvert: custom=" + conv.custom + " compareVal: " + compareValue + " conv.cap: " + conv.cap.toLocaleString() + " conv.discount: " + conv.discount + " discountPrice: " + discountPrice.toFixed(4) + " conv.price: " + conv.price.toFixed(4));

    return(conv);
}

/************************************************
 * Return a conv in which the totalShares is the total shares in the denominator based on the convert's definition
 * with the exception that the converts own shares will not be included even if requested - those shares will be
 * accounted for in the price by subtracting the investment from the numerator (algebra!).  The denominator text
 * field will also be returned in equationDenominator
 *
 * @param custom
 * @oaram index
 * @param preexisrtConvs
 * @param oSharesPost
 * @param totalFDpreShares
 * @param existingOptions
 */

function customBuildDenom(custom, index, preexistConvs, oSharesPost, totalFDpreShares, existingOptions) {

    var conv = Object.create(Convert);
    conv.totalShares = 0;
    conv.equationDenominator = "";

    var includeIssued = (custom.search("customDenomIssued") != -1);
    var includeUnissued = (custom.search("customDenomUnissued") != -1);
    var includeAdditional = (custom.search("customDenomAdditional") != -1);
    var includePreexist = (custom.search("customDenomPreexist") != -1);

    /* var includeIssued = ("customDenomIssued" in dict);
    var includeUnissued = ("customDenomUnissued" in dict);
    var includeAdditional = ("customDenomAdditional" in dict);
    var includePreexist = ("customDenomPreexist" in dict);*/

    if (includeIssued) {    // I am pretty sure I can't handle the case where this isn't checked
        conv.totalShares += totalFDpreShares;
        conv.equationDenominator = Number(totalFDpreShares.toFixed(0)).toLocaleString()  + " (FD Pre Shares)";
    }
    if (includeUnissued) {
        conv.totalShares += existingOptions;
        conv.equationDenominator +=  "+ " + Number(existingOptions.toFixed(0)).toLocaleString()  + " (existing options)";
    }

    if (includeAdditional) {
        conv.totalShares += oSharesPost - existingOptions;
        conv.equationDenominator +=  "+ " + Number((oSharesPost-existingOptions).toFixed(0)).toLocaleString()  + " (new options)";
    }

    if (includePreexist) {
        c = preexistCount(preexistConvs, index);
        conv.equationDenominator +=  c.equationDenominator;
        conv.totalShares += c.totalShares;
    }


    return(conv);

}

/****************************************************************
 *
 * @param premoneyShares
 * @param valuation
 * @param preOrPost
 * @returns {number}
 */

function sharePriceEquity(totalFDpreShares, totalConvShares,oSharesNew, valuation) {
    var price = 0;
    var equationNum = "";
    var equationDenom = "";



    price = valuation /  (totalFDpreShares + totalConvShares + oSharesNew);

    equationNum = valuation.toLocaleString() + " (pre val)";
    equationDenom = Number(totalFDpreShares.toFixed(0)).toLocaleString() + " (FD Pre Shares) + " +
        Number(totalConvShares.toFixed(0)).toLocaleString()  + " (converted shares) + " +
        Number(oSharesNew.toFixed(0)).toLocaleString()  + " (new options)";


    Equity.equationNumerator = equationNum;
    Equity.equationDenominator = equationDenom;

    // console.debug("sharePriceEquity: price=" + price);

    return(price);
}


function showdiv(id) {
    var _hidediv = null;

    if(_hidediv)
        _hidediv();
    var div = document.getElementById(id);
    div.style.display = 'block';
    hidediv = function () { div.style.display = 'none'; };
}
function addCommon() {

    $("#dialogAddCommon").dialog({
        resizable: false,
        height: 400,
        width: 400,
        modal: true,
        buttons: {
            Add: function() {
                proceed();
                $(this).dialog("close");
            },
            Cancel: function() {
                $(this).dialog("close");
            }
        }
    });

    function proceed(){
        var type = document.getElementById("newCommonType");
        var name = "", shares = 0;
        console.debug("Common type: " + type.value);
        if (type.value == "YC") {
            name = "YC";
            shares = $("#YCCommonShares").val();
            addRowCommonTable(type.value, name, shares, "");
        } else if (type.value == "Options") {
            /*
             * The option table is already on the page, just fill it in from the dialog
             */
            shares = $("#optionPoolSize").val();
            $("#optionPoolSizeForm").val(shares);
            $('#optionPoolSizeForm').number(true);

            shares = $("#grantedOptions").val();
            $("#grantedOptionsForm").val(shares);
            $('#grantedOptionsForm').number(true);

            shares = $("#promisedOptions").val();
            $("#promisedOptionsForm").val(shares);
            $('#promisedOptionsForm').number(true);

            shares = $("#unissuedOptions").val();
            $("#unissuedOptionsForm").val(shares);
            $('#unissuedOptionsForm').number(true);


            $("#optionsTable").css("display", "block");
        } else {
            name = $("#commonHolderName").val();
            shares = $("#commonHolderShares").val();
            addRowCommonTable(type.value, name, shares, "");
        }


    }
}

function addCommonSelector(selected) {
    if (selected.value == 'Founder' || selected.value == "Other") {
        $("#addCommonDiv").css("display","block");
        $("#addYCDiv").css("display","none");
        $("#addOptionsDiv").css("display","none");
    } else if (selected.value == 'YC') {
        $("#addYCDiv").css("display","block");
        $("#addCommonDiv").css("display","none");
        $("#addOptionsDiv").css("display","none");
    } else if (selected.value == 'Options') {
        $("#addOptionsDiv").css("display","block");
        $("#addCommonDiv").css("display","none");
        $("#addYCDiv").css("display","none");
    }

}

function addConvert() {

    $("#dialogAddConvert").dialog({
        resizable: false,
        height: 180,
        width: 400,
        modal: true,
        buttons: {
            Add: function() {
                proceed();
                $(this).dialog("close");
            },
            Cancel: function() {
                $(this).dialog("close");
            }
        }
    });

    function proceed(){
        var type = document.getElementById("newConvertType");
        console.debug("type: " + type.value);
        if (type.value == "Custom")
            getCustomInfo();
        else if (type.value == "Post SAFE")
            getPMSafeInfo();

        addRowConvertTable(type.value, "");
    }

    function getCustomInfo() {
        console.debug("getCustominfo");

        $("#dialogCustom").dialog({
            resizable: false,
            height: 400,
            width: 550,
            modal: true,
            buttons: {
                Okay: function() {
                    proceed();
                    $(this).dialog("close");
                }
            }
        });

        function proceed(){

            var convertNum = Number(document.getElementById("convertCount").value) - 1; /* count already incremented */
            updateCustomAttributes(convertNum);

        }
    }

    function getPMSafeInfo() {
        console.debug("getPMSafeinfo");

        $("#dialogPMSafe").dialog({
            resizable: false,
            height: 250,
            width: 550,
            modal: true,
            buttons: {
                Okay: function() {
                    proceed();
                    $(this).dialog("close");
                }
            }
        });

        function proceed(){

            var convertNum = Number(document.getElementById("convertCount").value) - 1; /* count already incremented */
            updatePMsafeAttributes(convertNum);

        }
    }

}




function addEquityRound() {

    angelJSerror("Sorry: multiple rounds are not yet supported!");
    $("#equityTable").css("display", "block");
    return;

    $("#dialogAddConvert").dialog({
        resizable: false,
        height: 180,
        width: 400,
        modal: true,
        buttons: {
            Add: function() {
                proceed();
                $(this).dialog("close");
            },
            Cancel: function() {
                $(this).dialog("close");
            }
        }
    });

    function proceed(){
        var type = document.getElementById("newConvertType");
        console.debug("type: " + type.value);
        if (type.value == "Custom")
            getCustomInfo();
        addRowConvertTable(type.value, "");
    }

    function getCustomInfo() {
        console.debug("getCustominfo");

        $("#dialogCustom").dialog({
            resizable: false,
            height: 400,
            width: 550,
            modal: true,
            buttons: {
                Okay: function() {
                    proceed();
                    $(this).dialog("close");
                }
            }
        });

        function proceed(){

            var convertNum = Number(document.getElementById("convertCount").value) - 1; /* count already incremented */
            updateCustomAttributes(convertNum);

        }
    }

}

function addFounder() {

    // angelJSerror("Sorry: founder equity is yet supported!");
    $("#founderName").val("");
    $("#founderShares").val(0);

    $("#dialogAddFounder").dialog({
        resizable: false,
        height: 220,
        width: 400,
        modal: true,
        buttons: {
            Add: function() {
                proceed();
                $(this).dialog("close");
            },
            Cancel: function() {
                $(this).dialog("close");
            }
        }
    });

    function proceed(){
        var name = $("#founderName").val();
        var shares = $("#founderShares").val();

        console.debug("addFounder: (name,shares)" + angelParens(name, shares));

        addRowFounderTable(name, shares,"");
        $("#founderTable").css("display", "block");
    }

}

function addRowFounderTable(name, shares, modNum) {
    var founderCount = Number($("#founderCount"+modNum).val());
    var table = document.getElementById("founderTable"+modNum);
    var row = table.insertRow(-1);

    var founderName   = 'founder' + founderCount + "Name" + modNum;
    var founderShares   = 'founder' + founderCount + "Shares" + modNum;

    row.insertCell(0).innerHTML = "<a href='javascript:onclick=deleteModelFounder(" + founderCount + ",\"" + name +  "\")'><img src='/img/delete.png' title='Delete Founder'></a>"+
                       "&nbsp &nbsp<input type='text' id='" + founderName + "' name='" + founderName + "' value='" + name + "'>";
    row.insertCell(1).innerHTML = "<input type='text' id='" + founderShares + "' name='" + founderShares + "' value='" + shares + "'>";

    $("#founderCount"+modNum).val(founderCount+1);
    $('#' + founderShares).number(true);


}

function addRowCommonTable(type, name, shares, modNum) {
    console.debug("addRowCommonTable: type / modNum: (name, shares): " + type + " / " + modNum + ": " + angelParens(name,shares));
    var founderCount = Number($("#founderCount"+modNum).val());
    var otherCommonCount = Number($("#otherCommonCount"+modNum).val());

    var table, commname, commshares, count, del, tit;


    if (type == "Founder") {
        table = document.getElementById("founderTable"+modNum);
        commname   = 'founder' + founderCount + "Name" + modNum;
        commshares   = 'founder' + founderCount + "Shares" + modNum;
        count = founderCount;
        del = "deleteModelFounder";
        tit = "Delete Founder";
        $("#founderTable").css("display", "block");
    } else if (type == "Options") {
        console.debug("addRowCommonTable: options -- SHOULD NEVER BE HERE");
    } else {
        table = document.getElementById("otherCommonTable"+modNum);
        commname   = 'commonHolder' + otherCommonCount + "Name" + modNum;
        commshares   = 'commonHolder' + otherCommonCount + "Shares" + modNum;
        count = otherCommonCount;
        del = "deleteModelCommon";
        tit = "Delete Common Holder";
        $("#otherCommonTable").css("display", "block");
    }
    body = table.tBodies[0];
    row = body.insertRow(-1);

    row.insertCell(0).innerHTML = "<a href='javascript:onclick=" + del + "(" + count + ",\"" + name +  "\")'><img src='/img/delete.png' title='" + tit + "'></a>" +
            "&nbsp &nbsp<input type='text' id='" + commname + "' name='" + commname + "' value='" + name + "'>";
    row.insertCell(1).innerHTML = "<input type='text' id='" + commshares + "' name='" + commshares + "' value='" + shares + "'>";


    if (type == "Founder")
        $("#founderCount"+modNum).val(count+1);
    else if (type == "Options")
        $("#optionsSpecified"+modNum).val(1);
    else
        $("#otherCommonCount"+modNum).val(count+1);


    $('#' + commshares).number(true);



}

function deleteModelFounder(founderNum, founderName) {

    console.debug("deleteModelFounder founderNum: " + founderNum);
    $("#founderToDelete").html(founderName);

    $("#dialogDeleteFounder").dialog({
        resizable: false,
        height: 200,
        width: 400,
        modal: true,
        buttons: {
            Delete: function() {
                proceed(founderNum);
                $(this).dialog("close");
            },
            Cancel: function() {
                $(this).dialog("close");
            }
        }
    });

    function proceed(founderNum){

        var founderCount = Number($("#founderCount").val());

        cleanUpOutput();        // start by clearing any model output

        /*
         * create a list of founders so we can recreate our list
         */
        var index = 0;
        for (var i = 0; i < founderCount; i++) {
            if (i == founderNum)    // we are deleting this one so skip it
                index = i+1;
            Equity.founderNames[i] = $('#founder' + index + 'Name').val();
            Equity.founderEquity[i] = $('#founder' + index + 'Shares').val();
            index++;
        }
        founderCount--; // one founder gone!
        /*
         * now, delete all the rows and add them back with the delete row gone
         */
        $("#founderTable tbody tr").remove();
        $("#founderCount").val(0);

        for (var i = 0; i < founderCount; i++) {
            addRowCommonTable("Founder", Equity.founderNames[i], Equity.founderEquity[i], "");

        }
    }

}

function modelOptionPool() {

    var pool = 0; var granted = 0; var promised = 0; var unissued = 0;

    $("#optionPoolSize").css("color", "black");
    pool = Number($("#optionPoolSize").val());
    granted = Number($("#grantedOptions").val());
    promised = Number($("#promisedOptions").val());
    unissued = pool - granted - promised;
    if (unissued < 0)
        unissued = 0;

    if (granted > pool) {
        $("#optionPoolSize").css("color", "red");
        $(".ui-dialog-buttonpane button:contains('Add')").button("disable");
    } else {
        Number($("#unissuedOptions").val(unissued));
        $(".ui-dialog-buttonpane button:contains('Add')").button("enable");
    }





}

function modelOptionPoolForm() {

    var pool = 0; var granted = 0; var promised = 0; var unissued = 0;

    console.debug("modelOptionPoolForm");

    $("#optionPoolSizeForm").css("color", "black");

    pool = Number($("#optionPoolSizeForm").val());
    granted = Number($("#grantedOptionsForm").val());
    promised = Number($("#promisedOptionsForm").val());
    unissued = pool - granted - promised;
    if (unissued < 0)
        unissued = 0;

    if (granted > pool) {
        $("#optionPoolSizeForm").css("color", "red");
        // $(".ui-dialog-buttonpane button:contains('Add')").button("disable");
    } else {
        Number($("#unissuedOptionsForm").val(unissued));
        // $(".ui-dialog-buttonpane button:contains('Add')").button("enable");
    }





}

function deleteModelCommon(commonNum, commonName) {

    console.debug("deleteModelCommon commonNum: " + commonNum);
    $("#commonHolderToDelete").html(commonName);

    $("#dialogDeleteCommonHolder").dialog({
        resizable: false,
        height: 200,
        width: 400,
        modal: true,
        buttons: {
            Delete: function() {
                proceed(commonNum);
                $(this).dialog("close");
            },
            Cancel: function() {
                $(this).dialog("close");
            }
        }
    });

    function proceed(commonNum){

        var commonCount = Number($("#otherCommonCount").val());
        console.debug("commonCountx=" + commonCount);

        cleanUpOutput();        // start by clearing any model output

        /*
         * create a list of common holders so we can recreate our list
         */
        var index = 0;
        for (var i = 0; i < commonCount; i++) {
            if (i == commonNum)    // we are deleting this one so skip it
                index = i+1;
            Equity.commonNames[i] = $('#commonHolder' + index + 'Name').val();
            Equity.commonEquity[i] = $('#commonHolder' + index + 'Shares').val();
            index++;
        }
        commonCount--; // one common holder gone!
        /*
         * now, delete all the rows and add them back with the delete row gone
         */
        $("#otherCommonTable tbody tr").remove();
        $("#otherCommonCount").val(0);

        console.debug("commonCount=" + commonCount);

        for (var i = 0; i < commonCount; i++) {
            addRowCommonTable("Other", Equity.commonNames[i], Equity.commonEquity[i], "");
        }
    }

}

function updatePMsafeAttributes(idNumber) {


    var custom = document.getElementById("CONV" + idNumber + "custom");
    var special = document.getElementById("CONV" + idNumber + "special");
    /*
     * get all the custom info and write back to the form to be saved.
     */
    custom.value = "customDenomPreexist";


    if (document.getElementById("pmsafeDenomUnissued").checked) {
        custom.value += ",customDenomUnissued";
        special.innerHTML += " [including unissued options] ";
    }
    if (document.getElementById("pmsafeDenomAdditional").checked) {
        custom.value += ",customDenomAdditional";
        special.innerHTML += " [including additional options] ";
    }


    console.debug("updatePMsafeAttributes - custom value: " + custom.value);

}

function updateCustomAttributes(idNumber) {


    var custom = document.getElementById("CONV" + idNumber + "custom");
    /*
     * get all the custom info and write back to the form to be saved.
     */
    custom.value = "";

    var compare = document.getElementById("customCompareCapAndVal").checked;

    if (compare)
        custom.value += "capAndVal";
    else
        custom.value += "capPrice";

    if (document.getElementById("customDenomIssued").checked)
        custom.value += ",customDenomIssued";
    /* the next two optional denom parts aren't yet supported since we won't likely have  that info */
    /*            if (document.getElementById("customDenomOutVest").checked)
     custom.value += "&customDenomOutVest";
     if (document.getElementById("customDenomOutUnvest").checked)
     custom.value += "&customDenomOutUnvest";*/
    if (document.getElementById("customDenomUnissued").checked)
        custom.value += ",customDenomUnissued";
    if (document.getElementById("customDenomAdditional").checked)
        custom.value += ",customDenomAdditional";
    if (document.getElementById("customDenomThis").checked)
        custom.value += ",customDenomThis";
    if (document.getElementById("customDenomPreexist").checked)
        custom.value += ",customDenomPreexist";


    console.debug("updateCustomAttributes - custom value: " + custom.value);

}

function updateCustomInfo(idNumber) {
    console.debug("updateCustominfo: idNumber=" + idNumber);
    /* TBD: update form from the values in this convert */

    $("#dialogCustom").dialog({
        resizable: false,
        height: 400,
        width: 550,
        modal: true,
        buttons: {
            Update: function() {
                updateCustomAttributes(idNumber);
                /* TBD: restore form to default values */
                $(this).dialog("close");
            }
        }
    });

}


function deleteConvert(idNumber) {
    // var tr = $(t).closest("tr")
    console.debug("deleteConvert idNumber: " + idNumber);

    $("#dialogDelete").dialog({
        resizable: false,
        height: 200,
        width: 400,
        modal: true,
        buttons: {
            Proceed: function() {
                proceed(idNumber);
                $(this).dialog("close");
            },
            Cancel: function() {
                $(this).dialog("close");
            }
        }
    });

    function proceed(idNumber){

        deleteRowConvertTable(idNumber);
    }

}
/*
 * delete all of the converts and re-add in order. just think this will be easier!
 */
function deleteRowConvertTable(idNumber) {

    var convertCount = Number(document.getElementById("convertCount").value);

    cleanUpOutput();        // start by clearing any model output

    createConvertList("");
    var table = document.getElementById("convertTable");

    ConvertList[idNumber].type = "";

    /* remove all converts */
    for (i=0; i < convertCount; i++) {
        table.deleteRow(-1);  // converts are at the end
    }
    document.getElementById("convertCount").value = 0;

    newIndex = 0;
    for (i=0; i <convertCount; i++) {

        conv = ConvertList[i];
        if (conv.type == "")
            continue;

        addRowConvertTable(conv.type, "");

        yourInv = document.getElementById("CONV"+newIndex+"convAmount");
        totalInv = document.getElementById("CONV"+newIndex+"convTotal");
        discount = document.getElementById("CONV"+newIndex+"discount");
        cap = document.getElementById("CONV"+newIndex+"cap");
        custom = document.getElementById("CONV"+newIndex+"custom");
        preexist = document.getElementById("CONV"+newIndex+"preexist");


        yourInv.value = conv.yourInvestment;
        totalInv.value = conv.totalInvested;
        discount.value = conv.discount;
        cap.value = conv.cap;
        custom.value = conv.custom;
        preexist.value = conv.preexist;

        newIndex++;
    }

    // now kill the js converts we created
    len = ConvertList.length;
    for (i=0; i < len; i++)
        ConvertList.pop();

}

function deleteRowConvertTable2(idNumber) {
    var convertCount = Number(document.getElementById("convertCount").value);
    var row = document.getElementById("CONV" + idNumber);
    var table = document.getElementById("convertTable");


    row.parentNode.removeChild(row);
    var rowCount = table.rows.length;

    /*
     * now renumber all existing converts
     */
    var table = document.getElementById("convertTable");
    var convNum;
    var type;
    for (i=1; i < rowCount; i++) {
        convNum = i-1;
        // type = table.rows[i].cells[4].childNodes[2].value;

        //console.debug("elements: " + table.rows[i].cells[4].childNodes.length);
        if (table.rows[i].cells[4].childNodes.length > 2) {                 /* it's a YCVC safe */
            table.rows[i].cells[1].childNodes[0].id = "CONV" + convNum + "convAmount";
            table.rows[i].cells[1].childNodes[0].name = "CONV" + convNum + "convAmount";
            // there's a hardcoded amount for convTotal which is the first child
            table.rows[i].cells[2].childNodes[1].id = "CONV" + convNum + "convTotal";
            table.rows[i].cells[2].childNodes[1].name = "CONV" + convNum + "convTotal";
            table.rows[i].cells[3].childNodes[0].id = "CONV" + convNum + "discount";
            table.rows[i].cells[3].childNodes[0].name = "CONV" + convNum + "discount";
            // there's also a hardcoded amount for the cap
            table.rows[i].cells[4].childNodes[1].id = "CONV" + convNum + "cap";
            table.rows[i].cells[4].childNodes[1].name = "CONV" + convNum + "cap";
            table.rows[i].cells[4].childNodes[2].id = "CONV" + convNum + "type";
            table.rows[i].cells[4].childNodes[2].name = "CONV" + convNum + "type";
        } else {
            table.rows[i].cells[1].childNodes[0].id = "CONV" + convNum + "convAmount";
            table.rows[i].cells[1].childNodes[0].name = "CONV" + convNum + "convAmount";
            table.rows[i].cells[2].childNodes[0].id = "CONV" + convNum + "convTotal";
            table.rows[i].cells[2].childNodes[0].name = "CONV" + convNum + "convTotal";
            table.rows[i].cells[3].childNodes[0].id = "CONV" + convNum + "discount";
            table.rows[i].cells[3].childNodes[0].name = "CONV" + convNum + "discount";
            table.rows[i].cells[4].childNodes[0].id = "CONV" + convNum + "cap";
            table.rows[i].cells[4].childNodes[0].name = "CONV" + convNum + "cap";
            table.rows[i].cells[4].childNodes[1].id = "CONV" + convNum + "type";
            table.rows[i].cells[4].childNodes[1].name = "CONV" + convNum + "type";
        }



    }
    document.getElementById("convertCount").value = convertCount-1;      // update convert count on form

}


function addRowConvertTable(type, modNum) {

    var cCount  = "convertCount"+modNum;
    var cTable  = "convertTable"+modNum;

    var convertCount = Number(document.getElementById("convertCount"+modNum).value);
    var table = document.getElementById("convertTable"+modNum);
    var rowCount = table.rows.length;
    var row = table.insertRow(rowCount);
    var convertNum = convertCount;

    console.debug("AddRowConvertTable: convertType = " + type + " convertCount = " + convertCount + " convertNum = " + convertNum + " modNum= " + modNum);

    // $( "#dialog" ).dialog();



    var convAmount  = 'CONV'+convertNum + "convAmount"+modNum;
    var convTotal   = 'CONV'+convertNum + "convTotal"+modNum;
    var discount    = 'CONV'+convertNum + "discount"+modNum;
    var cap         = 'CONV'+convertNum + "cap"+modNum;
    var convType    = 'CONV'+convertNum + "type"+modNum;
    var convCustom  = 'CONV'+convertNum + "custom"+modNum;
    var convSpecial  = 'CONV'+convertNum + "special"+modNum;
    // var convPMSafe  = 'CONV'+convertNum + "pmsafe"+modNum;
    var convPreexist  = 'CONV'+convertNum + "preexist"+modNum;
    var convShares        = 'CONV'+convertNum + "shares"+modNum;       // for manual only

    row.id = 'CONV'+convertNum;
    if (type == "Custom")
        row.insertCell(0).innerHTML = "<a href='javascript:onclick=deleteConvert(" + convertNum
                                + ")'><img src='/img/delete.png' title='Delete Convertible'></a>  ("+convertCount+") " +
                                "<a style='color:white' href='javascript:onclick=updateCustomInfo(" + convertNum + ")'> " + type + "</a>";
    else
        row.insertCell(0).innerHTML = "<a href='javascript:onclick=deleteConvert(" + convertNum + ")'><img src='/img/delete.png' title='Delete Convertible'></a>  ("+convertCount+") " + type;

    if (type == "YCVC SAFE") {
        row.insertCell(1).innerHTML= "<input type='hidden' id='" + convAmount + "' name='" + convAmount + "' value='0'>";
        row.insertCell(2).innerHTML= "100,000<input size='10%' type=hidden id='" + convTotal + "' name='" + convTotal+ "' value='100000'>";
        row.insertCell(3).innerHTML= "<input type='hidden' id='" + discount + "' name='" + discount + "' value='0'>";
        row.insertCell(4).innerHTML= "10,000,000<input type='hidden' id='" + cap + "' name='" + cap +"' value='10000000'>" +
            "<input type='hidden' id='" + convType + "' name='" + convType + "' value='" + type +"'>" +
            "<input type='hidden' id='" + convShares + "' name='" + convShares + "' value=''>" +
            "<input type='hidden' id='" + convCustom + "' name='" + convCustom + "' value=''>";
        row.insertCell(5).innerHTML= "<input size='15%' type=text id='" + convPreexist +
                            "' pattern='[+]?[0-9]*[.,]?[0-9]+' name='" + convPreexist + "'> " +
                            "<br /><span>List any preexisting convertibles by their numbers in the first column of this table. (e.g. '0,1')</span>";
    } else if (type == "YCII SAFE") {
        row.insertCell(1).innerHTML= "<input type='hidden' id='" + convAmount + "' name='" + convAmount + "' value='0'>";
        row.insertCell(2).innerHTML= "<input size='10%' type=text id='" + convTotal + "' pattern='[+]?[0-9]*[.,]?[0-9]+' name='" + convTotal + "'>";
        //row.insertCell(2).innerHTML= "$120,000<input size='10%' type=hidden id='" + convTotal + "' name='" + convTotal+ "' value='120000'>";
        row.insertCell(3).innerHTML= "<input type='hidden' id='" + discount + "' name='" + discount + "' value='0'>";
        row.insertCell(4).innerHTML= "Uncapped<input type='hidden' id='" + cap + "' name='" + cap +"' value='0'>"+
            "<input type='hidden' id='" + convType + "' name='" + convType + "' value='" + type +"'>" +
            "<input type='hidden' id='" + convShares + "' name='" + convShares + "' value=''>" +
            "<input type='hidden' id='" + convCustom + "' name='" + convCustom + "' value=''>" +
            "<input type='hidden' id='" + convPreexist + "' name='" + convPreexist + "' value=''>";
    } else if (type == "YC POST") {
        row.insertCell(1).innerHTML= "<input type='hidden' id='" + convAmount + "' name='" + convAmount + "' value='0'>";
        row.insertCell(2).innerHTML= "150,000<input size='10%' type=hidden id='" + convTotal + "' name='" + convTotal+ "' value='150000'>";
        row.insertCell(3).innerHTML= "<input type='hidden' id='" + discount + "' name='" + discount + "' value='0'>";
        row.insertCell(4).innerHTML= "N/A<input type='hidden' id='" + cap + "' name='" + cap +"' value='0'>"+
            "<input type='hidden' id='" + convType + "' name='" + convType + "' value='" + type +"'>" +
            "<input type='hidden' id='" + convShares + "' name='" + convShares + "' value=''>" +
            "<input type='hidden' id='" + convCustom + "' name='" + convCustom + "' value='customDenomUnissued'>" +
            "<input type='hidden' id='" + convPreexist + "' name='" + convPreexist + "' value=''>";
        row.insertCell(5).innerHTML= "<span>This convertible will always convert to 7% of the company's fully diluted shares just prior to an equity round, excluding any option pool increase associated with the round."
            + "</span>";
    } else if (type == "Custom") {
        row.insertCell(1).innerHTML= "<input size='10%' type=text id='" + convAmount + "' pattern='[+]?[0-9]*[.,]?[0-9]+' name='" + convAmount + "'>";
        row.insertCell(2).innerHTML= "<input size='10%' type=text id='" + convTotal + "' pattern='[+]?[0-9]*[.,]?[0-9]+' name='" + convTotal + "'>";
        row.insertCell(3).innerHTML= "<input size='10%' type=text id='" + discount + "' pattern='[+]?[0-9]*[.,]?[0-9]+' name='" + discount + "'>";
        row.insertCell(4).innerHTML= "<input size='10%' type=text id='" + cap + "' pattern='[+]?[0-9]*[.,]?[0-9]+' name='" + cap + "'>" +
            "<input type='hidden' id='" + convType + "' name='" + convType + "' value='" + type +"'>" +
            "<input type='hidden' id='" + convShares + "' name='" + convShares + "' value=''>" +
            "<input type='hidden' id='" + convCustom + "' name='" + convCustom + "' value=''>";
        row.insertCell(5).innerHTML= "<input size='10%' type=text id='" + convPreexist +
            "' pattern='[+]?[0-9]*[.,]?[0-9]+' name='" + convPreexist + "'> " +
            "<br /><span>List any preexisting convertibles by their numbers in the first column of this table. (e.g. '0,1')</span>";

    } else if (type == "Post SAFE") {
        row.insertCell(1).innerHTML= "<input size='10%' type=text id='" + convAmount + "' pattern='[+]?[0-9]*[.,]?[0-9]+' name='" + convAmount + "'>";
        row.insertCell(2).innerHTML= "<input size='10%' type=text id='" + convTotal + "' pattern='[+]?[0-9]*[.,]?[0-9]+' name='" + convTotal + "'>";
        row.insertCell(3).innerHTML= "<input size='10%' type=text id='" + discount + "' pattern='[+]?[0-9]*[.,]?[0-9]+' name='" + discount + "'>";
        row.insertCell(4).innerHTML= "<input size='10%' type=text id='" + cap + "' pattern='[+]?[0-9]*[.,]?[0-9]+' name='" + cap + "'>" +
                                      "<input type='hidden' id='" + convType + "' name='" + convType + "' value='" + type +"'>" +
            "<input type='hidden' id='" + convCustom + "' name='" + convCustom + "' value=''>" +
            "<input type='hidden' id='" + convPreexist + "' name='" + convPreexist + "' value=''>" +
            "<input type='hidden' id='" + convShares + "' name='" + convShares + "' value=''>";
        row.insertCell(5).innerHTML= "<span id='" + convSpecial + "'>This convertible's price is calculated using the number of fully diluted shares just prior to an equity round."
                                     + "</span>";
    } else if (type == "Manual") {
        row.insertCell(1).innerHTML= "<input size='10%' type=hidden id='" + convAmount + "' pattern='[+]?[0-9]*[.,]?[0-9]+' name='" + convAmount + "'>";
        row.insertCell(2).innerHTML= "<input size='10%' type=text id='" + convTotal + "' pattern='[+]?[0-9]*[.,]?[0-9]+' name='" + convTotal + "'>";
        row.insertCell(3).innerHTML= "<input size='10%' type=hidden id='" + discount + "' pattern='[+]?[0-9]*[.,]?[0-9]+' name='" + discount + "'>";
        row.insertCell(4).innerHTML= "<input size='10%' type=hidden id='" + cap + "' pattern='[+]?[0-9]*[.,]?[0-9]+' name='" + cap + "'>" +
            "<input type='hidden' id='" + convType + "' name='" + convType + "' value='" + type +"'>" +
            "<input type='hidden' id='" + convCustom + "' name='" + convCustom + "' value=''>" +
            "<input type='hidden' id='" + convPreexist + "' name='" + convPreexist + "' value=''>";
        row.insertCell(5).innerHTML= "<input size='10%' type=text id='" + convShares +
            "' pattern='[+]?[0-9]*[.,]?[0-9]+' name='" + convShares + "'> " +
            "<br /><span>Manual entry: give the number of shares this converted into</span>";

    }  else {
        row.insertCell(1).innerHTML= "<input size='10%' type=text id='" + convAmount + "' pattern='[+]?[0-9]*[.,]?[0-9]+' name='" + convAmount + "'>";
        row.insertCell(2).innerHTML= "<input size='10%' type=text id='" + convTotal + "' pattern='[+]?[0-9]*[.,]?[0-9]+' name='" + convTotal + "'>";
        row.insertCell(3).innerHTML= "<input size='10%' type=text id='" + discount + "' pattern='[+]?[0-9]*[.,]?[0-9]+' name='" + discount + "'>";
        row.insertCell(4).innerHTML= "<input size='10%' type=text id='" + cap + "' pattern='[+]?[0-9]*[.,]?[0-9]+' name='" + cap + "'>" +
            "<input type='hidden' id='" + convType + "' name='" + convType + "' value='" + type +"'>" +
            "<input type='hidden' id='" + convCustom + "' name='" + convCustom + "' value=''>" +
            "<input type='hidden' id='" + convShares + "' name='" + convShares + "' value=''>" +
            "<input type='hidden' id='" + convPreexist + "' name='" + convPreexist + "' value=''>";
    }

    document.getElementById(cCount).value = convertCount+1;      // update convert count on form
    $('#' + convAmount).number(true);
    $('#' + convTotal).number(true);
    $('#' + discount).number(true);
    $('#' + cap).number(true);
    $('#' + convShares).number(true);

    document.getElementById(cTable).style.display = "block";

}   // addrowconverttable()
/*
 * finish the angel ownership table by adding any equity ownership and a totals row.
 */
function addAngelEquityRowCapTable() {

    var table = document.getElementById("ownershipTable");
    var body = table.tBodies[0];
    var rowCount = table.rows.length;
    var cell;
    var percent=0;

    console.debug("addAngelEquityRowCapTable: rowCount = " + rowCount + " shares = " + Equity.yourShares);

    if (Equity.yourShares != 0) {

        var row = body.insertRow(-1);

        row.insertCell(0).innerHTML = "<span title='Show Price Calc'><a onClick='displayEquation(this,\"vcEquationDiv\")'>" + "You-equity" +  "</a></span>";
        row.insertCell(1).innerHTML= "<span>" + "$" + (Equity.price).toFixed(Precision.digits()) + "</span>";
        row.insertCell(2).innerHTML= "<span>" + Number(Equity.yourShares.toFixed(0)).toLocaleString() + "</span>";

        percent = (Equity.yourShares / Equity.postFDShares)*100;

        row.insertCell(3).innerHTML= "<span>" + percent.toFixed(Precision.digits()) + "%</span>";


        rowCount++;
    }

    return;
    /*
     * DEPRECATED - WE NEED TO DO THIS IN updatecaptable! bit of a hack, the table starts with 3 rows, if there are more than 4 that means there are 2 or more rows
     * which should be totaled.
     */
    if (rowCount > 4) {
        var row = table.insertRow(rowCount);    // add hr
        row.insertCell(0).innerHTML = "<hr>";
        row.cells[0].colSpan = 4;
        rowCount++;

        var totalShares = 0;
        for (i=0; i < ConvertList.length; i++) {
            conv = ConvertList[i];
            totalShares += conv.yourShares;
        }
        totalShares += Equity.yourShares;

        var row = table.insertRow(rowCount);

        row.insertCell(0).innerHTML = "<span style='color:black'>" + "Your Total" +  "</span>";
        row.insertCell(1).innerHTML= "<span>" + "</span>";
        row.insertCell(2).innerHTML= "<span>" + Number(totalShares.toFixed(0)).toLocaleString() + "</span>";

        percent = (totalShares / Equity.postFDShares)*100;

        row.insertCell(3).innerHTML= "<span>" + percent.toFixed(4) + "%</span>";
    }


}

/*
 * add a single row to the angel table for the input convertible.
 */
function addAngelConvertRowCapTable(conv, convertNum) {

    var table = document.getElementById("ownershipTable");
    var body = table.tBodies[0];
    var rowCount = table.rows.length;
    var cell;
    var percent=0;

    // console.debug("AddConvertRowAngelTable: rowCount = " + rowCount + " convertNum = " + convertNum + " shares = " + conv.yourShares);

    if (conv.yourShares == 0)
        return;

    var row = body.insertRow(-1);

    var divName = 'CONV' + convertNum + "EquationDiv";
    row.insertCell(0).innerHTML = "<span title='Show Price Calc'><a onClick='displayEquation(this,\"" + divName + "\")'>" + "You-" + convertNum + " (" + Number(conv.cap/1000000).toFixed(0) + "mm)" + "</a></span>";
    row.insertCell(1).innerHTML= "<span>" + "$" + (conv.price).toFixed(Precision.digits()) + "</span>";
    row.insertCell(2).innerHTML= "<span>" + Number(conv.yourShares.toFixed(0)).toLocaleString() + "</span>";

    percent = (conv.yourShares / Equity.postFDShares)*100;

    row.insertCell(3).innerHTML= "<span>" + percent.toFixed(Precision.digits()) + "%</span>";


}
/*
 * add converts to cap table
 */
function addConvertRowCapTable(conv, convertNum) {

    var table = document.getElementById("convertOwnershipTable");
    var body = table.tBodies[0];
    // var equationTable = document.getElementById("equationTable");
    var rowCount = table.rows.length;
    var row = body.insertRow(-1);
    var type=conv.type;    // just support safes for now
    var cell;
    var percent=0;

    console.debug("AddConvertRowCapTable: rowCount = " + rowCount + " convertNum = " + convertNum);

    var divName = 'CONV' + convertNum + "EquationDiv";

    cell = row.insertCell(0);
    cell.innerHTML = "<span title='Show Price Calc'><a onClick='displayEquation(this,\"" + divName + "\")'>" + type + "-" + convertNum + " (" + Number(conv.cap/1000000).toFixed(1) + "mm)" + "</a></span>";
    cell.className = "showCalc";
    cell.style.textAlign = "right";
    row.insertCell(1).innerHTML= "<span id='" + type + convertNum + "Price'>" + "$" + (conv.price).toFixed(Precision.digits()) + "</span>";
    row.insertCell(2).innerHTML= "<span id='" + type + convertNum + "Shares'>" + Number(conv.totalShares.toFixed(0)).toLocaleString() + "</span>";

    percent = (conv.totalShares / Equity.postFDShares)*100;

    row.insertCell(3).innerHTML= "<span id='" + type + convertNum + "Percent'>" + percent.toFixed(Precision.digits()) + "%</span>";


}


/*
 * add common row to cap table (founders or other) while modeling
 */
function addCommonRowCapTable(tableName, name, shares) {

    var table = document.getElementById(tableName);
    var body = table.tBodies[0];;
    var rowCount = table.rows.length;
    var row = body.insertRow(-1);
    var cell;
    var percent=0;

    // console.debug("AddCommonRowCapTable: rowCount = " + rowCount + angelParens(name,shares));


    cell = row.insertCell(0);
    if (tableName == "founderOwnershipTable")
        cell.innerHTML = "<span>" + name + " (founder)</span>";
    else
        cell.innerHTML = "<span>" + name + "</span>";

    cell.style.textAlign = "right";
    row.insertCell(1).innerHTML= "N/A";     // price is not applicable
    row.insertCell(2).innerHTML= "<span>" + shares.toLocaleString() + "</span>";

    percent = (shares / Equity.postFDShares)*100;

    row.insertCell(3).innerHTML= "<span>" + percent.toFixed(Precision.digits()) + "%</span>";

}

function deleteRow(obj) {
    var index = obj.parentNode.parentNode.rowIndex;
    var table = document.getElementById("myTableData");
    table.deleteRow(index);
}

function updateFinancingSummary(stage) {
    /*
     * stage is initial (before converts), pre (pre equity), post (post equity)
     */

    if (stage == "initial") {
        $("#financingSummaryTitle").html("Initial Capitalization Summary");
    } else if (stage == "pre") {
        $("#financingSummaryTitle").html("Pre-Equity Financing Summary");
    } else
        $("#financingSummaryTitle").html("Post-Equity Financing Summary");

    var postVal = document.getElementById('postVal');
    var postShares = document.getElementById('postFDShares');
    var postPrice = document.getElementById('postPrice');
    var postOptions = document.getElementById('postOptions');
    var postOptionsPercent = document.getElementById('postOptionsPercent');
    var postVCOwnership = document.getElementById('postVCOwnership');
    var postCommonOwnership = document.getElementById('postCommonOwnership');
    var postConvOwnership = document.getElementById('postConvOwnership');
    var postOwnershipTotal = document.getElementById('postOwnershipTotal');

    var ownershipTotal = 0;

    var optionsTotal = Equity.postOptions + Equity.prePromisedOptions + Equity.preGrantedOptions;

    var postValuation = Equity.preMoneyVal + Equity.totalInvested;
    var availOptionOwnership  = Equity.postOptions / Equity.postFDShares;
    var optOwnership  = optionsTotal / Equity.postFDShares;

    if (stage == "post") {
        postVal.innerHTML       = "$" + postValuation.toLocaleString();
    } else
        postVal.innerHTML       = "N/A";

    postOptions.innerHTML   = Number(Equity.postOptions.toFixed(0)).toLocaleString() + " (" + (availOptionOwnership*100).toFixed(2) + "%" + ")";

    postShares.innerHTML    = Number(Equity.postFDShares.toFixed(0)).toLocaleString();
    var newPrice = Precision.round(postValuation/Equity.postFDShares);
    postPrice.innerHTML     = "$" + String(Equity.price.toFixed(Precision.digits()));


    postOptionsPercent.innerHTML = (optOwnership*100).toFixed(Precision.digits()) + "%";

    ownershipTotal += optOwnership;

    var vcOwnership  = Precision.round(Equity.totalShares / Equity.postFDShares);
    postVCOwnership.innerHTML = (vcOwnership*100).toFixed(Precision.digits()) + "%";

    ownershipTotal += vcOwnership;


    /*
     * then all the converts
     */
    var totalConvShares = 0;
    for (i=0; i < ConvertList.length; i++) {
        conv = ConvertList[i];
        totalConvShares += conv.totalShares;
    }
    var convOwnership = totalConvShares / Equity.postFDShares;
    postConvOwnership.innerHTML = (convOwnership*100).toFixed(Precision.digits()) + "%";

    ownershipTotal += convOwnership;

    /*
     * then all the founders and other common
     */
    var preOptionsTotal = Equity.preGrantedOptions + Equity.prePromisedOptions + Equity.preUnallocOptions;
    var commonOwnership  = (Equity.preFDShares - preOptionsTotal) / Equity.postFDShares;
    var founderOwnership = 0;
    if (Equity.numFounders > 0) {
        founderOwnership = Equity.founderTotalEquity / Equity.postFDShares;
        commonOwnership -= founderOwnership;        // founder shares are part of common

       $('#postFounderOwnership').html((founderOwnership*100).toFixed(Precision.digits()) + "%");
    } else
        $('#postFounderOwnership').html("");


    postCommonOwnership.innerHTML = (commonOwnership*100).toFixed(Precision.digits()) + "%";

    ownershipTotal += commonOwnership+founderOwnership;


    postOwnershipTotal.innerHTML = (ownershipTotal*100).toFixed(Precision.digits()) + "%";

    if (stage == "post") {
        // display pre-money fd shares and new shares
        var newSharePercent = (Equity.postFDShares-Equity.preFDShares)/Equity.postFDShares;

        $("#preFDShares").html(Number(Equity.preFDShares.toFixed(0)).toLocaleString());
        $("#newShares").html(Number((Equity.postFDShares-Equity.preFDShares).toFixed(0)).toLocaleString() +
                                    " (" + (newSharePercent*100).toFixed(2) + "%" + ")");

        $("#preFDSharesRow").css("display","table-row");
        $("#newSharesRow").css("display","table-row");
    } else {
        $("#preFDSharesRow").css("display","none");
        $("#newSharesRow").css("display","none");
    }


}

function updateCapTable(stage) {
    /*
     * stage is initial (before converts), pre (pre equity), post (post equity)
     */
    var totalConvShares = 0;
    var totalYourShares = 0;
    var totalPreferredPlusOptions = 0;


    if (stage == "initial") {
        $("#capTableTitle").html("Initial Cap Table");
    } else if (stage == "pre") {
        $("#capTableTitle").html("Pre-Equity Cap Table");
    } else
        $("#capTableTitle").html("Post-Equity Cap Table");
    /*
     * Add this investor to the cap table
     */
    for (i=0; i < ConvertList.length; i++) {
        conv = ConvertList[i];
        totalYourShares += conv.yourShares;
        addAngelConvertRowCapTable(conv, i);  // second argument is convert number
    }
    addAngelEquityRowCapTable();
    totalYourShares += Equity.yourShares;

    $("#yourPrice").html("");
    $("#yourShares").html(Number(totalYourShares.toFixed(0)).toLocaleString());   // options not added yet! just totalpreferred
    $("#yourOwnership").html((totalYourShares/Equity.postFDShares*100).toFixed(Precision.digits())+"%");

    /*
     * Add the preferred holders Starting with the VC
     */
    var vcOwn = document.getElementById('vcOwnership');
    var vcShr = document.getElementById('vcShares');
    var vcPri = document.getElementById('vcPrice');

    var vcOwnership  = Equity.totalShares / Equity.postFDShares;
    vcOwn.innerHTML = (vcOwnership*100).toFixed(Precision.digits()) + "%";
    vcPri.innerHTML = "$" + String(Equity.price.toFixed(Precision.digits()));
    vcShr.innerHTML = Number(Equity.totalShares.toFixed(0)).toLocaleString();

    totalPreferredPlusOptions += Equity.totalShares;
    //console.debug("totalPreferredPlusOptions (1) = " + totalPreferredPlusOptions);

    /*
     * then all the converts
     */

    for (i=0; i < ConvertList.length; i++) {
        conv = ConvertList[i];
        addConvertRowCapTable(conv, i);  // second argument is convert number
        totalConvShares += conv.totalShares;
    }
    totalPreferredPlusOptions += totalConvShares;
    //console.debug("totalPreferredPlusOptions (2) = " + totalPreferredPlusOptions);

    $("#preferredPrice").html("");
    $("#preferredShares").html(Number(totalPreferredPlusOptions.toFixed(0)).toLocaleString());   // options not added yet! just totalpreferred
    $("#preferredOwnership").html((totalPreferredPlusOptions/Equity.postFDShares*100).toFixed(Precision.digits())+"%");

    /*
     * Then the Options Section
     */
    var optOwn = document.getElementById('availableOptionsOwnership');
    var optShr = document.getElementById('availableOptionsShares');

    var optOwnership  = Equity.postOptions / Equity.postFDShares;
    optOwn.innerHTML = (optOwnership*100).toFixed(Precision.digits()) + "%";
    optShr.innerHTML = Number(Equity.postOptions.toFixed(0)).toLocaleString();

    var optOwnership  = Equity.prePromisedOptions / Equity.postFDShares;
    $("#promisedOptionsOwnership").html((optOwnership*100).toFixed(Precision.digits()) + "%");
    $("#promisedOptionsShares").html(Number(Equity.prePromisedOptions.toFixed(0)).toLocaleString());

    var optOwnership  = Equity.preGrantedOptions / Equity.postFDShares;
    $("#grantedOptionsOwnership").html((optOwnership*100).toFixed(Precision.digits()) + "%");
    $("#grantedOptionsShares").html(Number(Equity.prePromisedOptions.toFixed(0)).toLocaleString());

    var totalOptions = Equity.preGrantedOptions+Equity.prePromisedOptions+Equity.postOptions;
    var optOwnership  = totalOptions / Equity.postFDShares;
    $("#optionsOwnership").html((optOwnership*100).toFixed(Precision.digits()) + "%");
    $("#optionsShares").html(Number(totalOptions.toFixed(0)).toLocaleString());

    totalPreferredPlusOptions += Equity.postOptions+Equity.preGrantedOptions+Equity.prePromisedOptions;
    //console.debug("totalPreferredPlusOptions (3) = " + totalPreferredPlusOptions);


    /*
     * Now add founders and remaining common
     */
    for (i=0; i < Equity.numFounders; i++) {
        // console.debug("updatecaptable: " + JSON.stringify(Equity.founderNames));
        var name = Equity.founderNames[i];
        var shares = Equity.founderEquity[i];
        addCommonRowCapTable("founderOwnershipTable", name, shares);
    }

    for (i=0; i < Equity.numOtherCommon; i++) {
        // console.debug("updatecaptable: " + JSON.stringify(Equity.founderNames));
        name = Equity.commonNames[i];
        shares = Equity.commonEquity[i];
        addCommonRowCapTable("otherCommonOwnershipTable", name, shares);
    }

    var coOwn = document.getElementById('commonOwnership');
    var coShr = document.getElementById('commonShares');
    var coPri = document.getElementById('commonPrice');

    var commonShares = Equity.postFDShares - totalPreferredPlusOptions;

    var coOwnership  = commonShares / Equity.postFDShares;
    coOwn.innerHTML = (coOwnership*100).toFixed(Precision.digits()) + "%";
    //coPri.innerHTML = "$" + String(Equity.price.toFixed(Precision.digits()));
    coPri.innerHTML = "";
    coShr.innerHTML = Number(commonShares.toFixed(0)).toLocaleString();

    var otherCommonShares = commonShares-Equity.commonTotalEquity-Equity.founderTotalEquity;
    $("#otherCommonPrice").html("N/A");
    $("#otherCommonShares").html(Number(otherCommonShares).toLocaleString());
    $("#otherCommonOwnership").html((otherCommonShares/Equity.postFDShares*100).toFixed(Precision.digits())+"%");
}


function updateEquationTable() {

    /*
     * You first
     */
    var yourNum = document.getElementById('your0EquationNum');
    var yourDen = document.getElementById('your0EquationDenom');


    if (ConvertList.length > 0) {
        conv = ConvertList[0];  // for now we only look at the default conversion. Eventually we'll get them all
        yourNum.innerHTML = conv.equationNumerator;
        yourDen.innerHTML = conv.equationDenominator;
    }

    /*
     * Then the VC
     */
    var vcNum = document.getElementById('vcEquationNum');
    var vcDen = document.getElementById('vcEquationDenom');

    vcNum.innerHTML = Equity.equationNumerator;
    vcDen.innerHTML = Equity.equationDenominator;


    /*
     * then all the converts
     */
    var safeNum;
    var safeDen;

    convHTML  = '<td  width="400px" id="CONV0EquationDenom">Denominator</td></tr></table></div>';

    for (i=0; i < ConvertList.length; i++) {
        var table = document.getElementById("equationTable");
        var body = table.tBodies[0];
        var row = body.insertRow(-1);

        conv = ConvertList[i];

        convHTML  = '<div class="angelEquationDiv" id="CONV' + i + 'EquationDiv" style="display:none"><span style="text-align: left; color:#1574B1">Convert Calculation</span><table class="angelEquationTable" style="width:300px"><tr><td style="width:50px"></td>';
        convHTML += '<td style="width:200px" id="CONV' + i + 'EquationNum">' + conv.equationNumerator + '</td></tr><tr><td id="angelPriceEquals" style="text-align: right; width:50px; padding-right: 10px">price =</td><td><hr></td></tr><tr><td></td>';
        convHTML += '<td id="CONV' + i + 'EquationDenom">' + conv.equationDenominator + '</td></tr></table></div>';
        row.insertCell(0).innerHTML =  convHTML;

       /* id = 'CONV'+i+'EquationNum';
        safeNum = document.getElementById(id);
        id = 'CONV'+i+'EquationDenom';
        safeDen = document.getElementById(id);

        safeNum.innerHTML = conv.equationNumerator;
        safeDen.innerHTML = conv.equationDenominator;*/

    }

}

function  updateForm() {
    var yourOwn = document.getElementById('yourOwnershipForm');
    var yourShr = document.getElementById('yourSharesForm');
    var yourPri = document.getElementById('yourPriceForm');
    var postShr = document.getElementById('postFDSharesForm');
    var postPri = document.getElementById('postPriceForm');

    if (ConvertList.length > 0) {
        var conv = ConvertList[0];

        yourOwn.value = conv.yourShares/Equity.postFDShares;
        yourShr.value = conv.yourShares;
        yourPri.value = conv.price;
    }
    postShr.value = Equity.postFDShares;
    postPri.value = Equity.price;

}
/************************************************************
 *
 * @param totalFDpreShares
 * @returns {number}
 */

function calcCapTable(totalFDpreShares) {

    // input to calculations

    var oSharesPost;

    var convertPrice;
    var otherConvertShares;
    var totalConvShares = 0;
    var totalConvSharesP1 = 0;
    var totalConvSharesP2 = 0;
    var vcPrice;
    var vcShares;
    var equityDenominator;
    var convDenominator;
    var oSharesPre;
    var oSharesNew;
    var postOptionsPercent = Equity.postOptionsPercent/100;
    //var preOptionsPercent = Equity.postOptionsPercent/100;
    var discount = Equity.discount / 100;

    var count;

    /*
     * take as your initial value the post options percent of fully diluted pre money shares * equity only share increase
     */
    // console.log("calccaptable totalFDpreShares: " + totalFDpreShares + " Equity: " + JSON.stringify(Equity));
    if (postOptionsPercent != 0)
        oSharesPost = Precision.round(postOptionsPercent * (totalFDpreShares * (1+(Equity.totalInvested/(Equity.totalInvested+Equity.preMoneyVal)))));
    else
        oSharesPost = Precision.round(.001 * (totalFDpreShares * (1+(Equity.totalInvested/(Equity.totalInvested+Equity.preMoneyVal)))));

    Equity.postFDShares = totalFDpreShares + oSharesPost;
    // in case we accidentally have too accurate a number before we start, make it less accurate!
    if ((oSharesPost/Equity.postFDShares - postOptionsPercent) <= .0000000001)
        oSharesPost = .90*oSharesPost;

    count = 0;
    // console.debug("***********CalcCapTable Enter: oSharesPost=" + oSharesPost + " percent of FD:" + oSharesPost/Equity.postFDShares);
    while (Math.abs(oSharesPost/Equity.postFDShares - postOptionsPercent) > .0000000001 && count < 100) {

        /*
         * next try at oSharesPost is as the proper percent of fully diluted post shares. Then recalc that.
         */

        oSharesPost = Precision.round(Equity.postFDShares * postOptionsPercent);

        oSharesPre = Equity.preUnallocOptions;
        oSharesNew = oSharesPost - oSharesPre;

        /*
         *  Second goal is to convert the converts until the equity price hardly changes any more - which
         *  will mean that the number of shares allocated to equity no longer changes (ignoring fractional shares)
         *  Or, if just modeling converts, the number of shares allocated to converts no longer changes
         */
        var price1=0, price2=0,priceCount = 0;

        do {
            /*
             *  calculate equity price before and after conversion of converts and test how much that
             *  price changed. when it changes sufficiently little we are done.
             */
            price1 = sharePriceEquity(totalFDpreShares,totalConvShares, oSharesNew, Equity.preMoneyVal);
            price1 = Precision.round(price1);
            totalConvShares = convertConverts(oSharesPost, totalFDpreShares, oSharesPre, price1);
            totalConvSharesP1 = Precision.round(totalConvShares).toFixed(0);
            price2 = sharePriceEquity(totalFDpreShares,totalConvShares, oSharesNew, Equity.preMoneyVal);
            price2 = Precision.round(price2);
            totalConvShares = Precision.round(totalConvShares);

            /*
             * if modeling converts only (price is 0) - calc converts a second time
             * in case there are post money safes
             */
            if (price2 == 0) {
                totalConvSharesP2 = convertConverts(oSharesPost, totalFDpreShares, oSharesPre, price1);
                totalConvSharesP2 = Precision.round(totalConvSharesP2).toFixed(0);
            }



            Equity.price = price2;

            Equity.totalShares = Precision.round(Equity.totalInvested / Equity.price);
            Equity.yourShares = Precision.round(Equity.yourInvestment / Equity.price);
            totalSharesP1 = Precision.round((Equity.totalInvested / price1).toFixed(0));

            Equity.postFDShares = Equity.preFDShares - Equity.preUnallocOptions + totalConvShares + Equity.totalShares + oSharesPost;
            /*console.log(">>>>>>CalcCapTable: getting price: price1=" + price1 + " price2=" + price2
                                        + " pricecount=" + priceCount + " totalSharesP1= " + totalSharesP1 + " totalShares=" + Equity.totalShares.toFixed(0));*/
            priceCount++;

        } while ((totalSharesP1 != Equity.totalShares.toFixed(0)  && priceCount < 100) || (totalConvSharesP1 != totalConvSharesP2 && priceCount < 100));


        count++;
        // console.log("***********CalcCapTable: count= "+ count + " totalFDPreShares=" + totalFDpreShares +" Shares=" + Equity.postFDShares + " oSharesPost=" + oSharesPost.toFixed(0) + " totalConvShares=" + totalConvShares.toFixed(0) + " vcShares=" + Equity.totalShares.toFixed(0) + " percent of FD:" + (oSharesPost/Equity.postFDShares).toFixed(5));

    }
    Equity.postOptions = oSharesPost;
    return(oSharesPost);

}
/************************************************************************
 *  Display an equation div with the indicated id via a click on the input element (which we will highlight)
 *
 * @param element
 * @param id
 */
function displayEquation(element, id) {

    /*
     * first hide all other equations
     */
    $('#angelEquationDiv').hide();
    $('#vcEquationDiv').hide();

    for (i=0; i < ConvertList.length; i++)
        $('#CONV'+i+'EquationDiv').hide();

    /* then show the correct / input one */

    $('#'+id).show();


    /*
     * highlight this show calc and turn hightlighting off all the rest
     */
    $('#capTable a:last-child').each(function() {
        $(this).css('color', '#1574B1');
        $(this).css('font-weight', 'normal');
    });

    /*$('#ownershipTable td:nth-child(1)').each(function() {
        $(this).children().css('color', '#1574B1');
    });*/
    //$("span").closest('td').css("backgroundColor", "white");
    // console.debug("displayEquation: " + angelParens(JSON.stringify(element.id), td));
   // td.style.backgroundColor = "white";
    element.style.color = "black";
    element.style.fontWeight = "bold";

}


function saveModel(n) {
    var form = document.getElementById("convertForm");
    var name = document.getElementById('modelName');
    var newName = document.getElementById('modelNameForm').innerHTML;

    var vcInv = document.getElementById("investAmount");
    var angelInv = document.getElementById('yourAmount');
    var val = document.getElementById("valuation");
    var valispre = document.getElementById("valispre");
    var postopt = document.getElementById("opoolPost");
    //var preopt = document.getElementById("opoolPre");
    var shares = document.getElementById("sharesOut");
    var numConverts = document.getElementById("convertCount").value;
    var numFounders = document.getElementById("founderCount").value;
    var numOtherCommon = document.getElementById("otherCommonCount").value;

    var optionPoolSize = document.getElementById("optionPoolSizeForm");
    var grantedOptions = document.getElementById("grantedOptionsForm");
    var promisedOptions = document.getElementById("promisedOptionsForm");
    var unissuedOptions = document.getElementById("unissuedOptionsForm");


    console.debug("---SaveModel: n= " + n + " name=" +name.value + " newname=" + newName + " numconverts=" + numConverts);

    if (n == "" && name.value=="") {
        var modelName = prompt("Please name your model");

        if (modelName == null)
            return;
    } else {
        if (n == "")
            modelName = name.value;
        else
            modelName = n;
    }


    if (newName != "" && newName.length < 20) {
        modelName = newName;
    }

    /*
     * the numbers are formatted thanks to the jquery number formatter so we need to grab the number value to submit
     */
    angelInv.value = $('#yourAmount').val();
    vcInv.value = $('#investAmount').val();
    val.value = $("#valuation").val();
    // preopt.value = $("#opoolPre").val();
    postopt.value = $("#opoolPost").val();
    // shares.value = $("#sharesOut").val();

    optionPoolSize.value = $("#optionPoolSizeForm").val();
    grantedOptions.value = $("#grantedOptionsForm").val();
    promisedOptions.value = $("#promisedOptionsForm").val();
    unissuedOptions.value = $("#unissuedOptionsForm").val();


    // console.debug("poolsize value: " + $("#optionPoolSizeForm").val());


    for (var i=0; i <numConverts; i++) {

        document.getElementById("CONV"+i+"convAmount").value = $("#CONV"+i+"convAmount").val();
        document.getElementById("CONV"+i+"convTotal").value = $("#CONV"+i+"convTotal").val();
        document.getElementById("CONV"+i+"discount").value = $("#CONV"+i+"discount").val();
        document.getElementById("CONV"+i+"cap").value = $("#CONV"+i+"cap").val();
        document.getElementById("CONV"+i+"shares").value = $("#CONV"+i+"shares").val();
    }

    for (i=0; i <numFounders; i++) {
        document.getElementById("founder"+i+"Shares").value = $("#founder"+i+"Shares").val();
    }

    for (i=0; i <numOtherCommon; i++) {
        document.getElementById("commonHolder"+i+"Shares").value = $("#commonHolder"+i+"Shares").val();
    }


    name.value = modelName;

    form.submit();

}
function loadModel(selected) {
    /*
     * grab the selected model id
     * send it to investment with a request for data
     * grab the data and put it into the forms (and potentially add new convert rows)
     */
    console.debug("loadModel: selected value is: " + selected.value);
    var share = document.getElementById('modelShareDiv');
    var sharing;


    clearModel("");
    clearSharingLink();
    if (selected.value != 0) {
        /*
         * this is a request for a model
         */
        if (selected.value == -1) {
            loadSampleModel("");
        } else {
            loadModelByID(selected.value, "", "");

        }

    }


}

function loadModelByID(modelID, modNum, sharedHash) {
    /*
     * grab the selected model id
     * send it to investment with a request for data
     * grab the data and put it into the forms (and potentially add new convert rows)
     */
    console.debug("loadModelByID: ID is: " + modelID + " modNum: " +  modNum + " sharedHash: " + sharedHash);

    var name = document.getElementById('modelName' + modNum);
    if (modNum == "")    // if modNum is blank this is not the compare page
        var editName = document.getElementById('modelNameForm' + modNum);

    var id = document.getElementById('modelID'+ modNum);

    var yourInv;
    var totalInv;
    var discount;
    var cap;
    var custom;
    var preexist;

    var vcInv = document.getElementById("investAmount"+modNum);
    var angelInv = document.getElementById("yourAmount"+modNum);
    var val = document.getElementById("valuation"+modNum);
    var valispre = document.getElementById("valispre"+modNum);
    var preOptionPoolSize = document.getElementById("optionPoolSizeForm"+modNum);
    var preGranted = document.getElementById("grantedOptionsForm"+modNum);
    var prePromised = document.getElementById("promisedOptionsForm"+modNum);
    var preUnissued = document.getElementById("unissuedOptionsForm"+modNum);
    var postopt = document.getElementById("opoolPost"+modNum);
    var shares = document.getElementById("sharesOut"+modNum);

    var numConverts = 0;
    var numFounders = 0;
    var numOtherCommon = 0;

    var url = "model?loadModel="+modelID;
    if (sharedHash != "")
        url += "&sharing="+sharedHash;

    $.get(url, function(data,status) {
        // alert("Data: " + data + "\nStatus: " + status)

        var dict = parseQueryStringToDictionary(data);
        var type;
        var sharing;
        var poolsize, unissued;

        // console.debug("modelID: " + dict["modelID"]);


        vcInv.value = $.number(dict["vcInvestment"]);
        angelInv.value = $.number(dict["yourInvestment"]);
        val.value = $.number(dict["preValuation"]);
        valispre.checked = true;
        postopt.value = dict["postOptionsPercent"];
        // postopt.value = $.number(dict["postOptionsPercent"]);
        poolsize = Number(dict["preOptionPoolSize"]);
        unissued = Number(dict["preUnissuedOptions"]);

        // console.debug("preOptionPoolSize: " + poolsize);
        preOptionPoolSize.value = $.number(dict["preOptionPoolSize"]);
        preGranted.value = $.number(dict["preGrantedOptions"]);
        prePromised.value = $.number(dict["prePromisedOptions"]);
        preUnissued.value = $.number(dict["preUnissuedOptions"]);

        // console.debug("loadmodelbyid: unissued.value: " + preUnissued.value);

        // shares.value = $.number(dict["fdPreMoneyShares"]);
        numConverts = $.number(dict["numberConverts"]);
        numFounders = $.number(dict["numberFounders"]);
        numOtherCommon = $.number(dict["numberOtherCommon"]);

        // if there is a pool or (legacy) unissued was set - previously w/equity display options
        if (poolsize > 0 || unissued > 0) {
            $("#optionsTable"+modNum).css("display", "block");
        } else
            $("#optionsTable"+modNum).css("display", "none");

        $("#optionPoolSizeForm"+modNum).number(true);
        $("#grantedOptionsForm"+modNum).number(true);
        $("#promisedOptionsForm"+modNum).number(true);
        $("#unissuedOptionsForm"+modNum).number(true);




        if (numFounders > 0) {
            var founderDict = parseQueryStringToDictionary(dict["founders"]);

            for (var n in founderDict) {
                addRowCommonTable("Founder", n, founderDict[n],modNum);
            }

        }
        if (numOtherCommon > 0) {
            var otherCommonDict = parseQueryStringToDictionary(dict["otherCommon"]);

            for (var n in otherCommonDict) {
                addRowCommonTable("Other", n, otherCommonDict[n],modNum);
            }

        }

        /*
         * If a share:
         *   If we don't name the model it will force the user to choose another name on save, which seems smart.
         *   Also, don't set the if a share - otherwise a save will overwrite it!
         */
        if (sharedHash == "") {
            name.value = dict["name"];
            if (modNum == "")       // edit name is not on compare page
                editName.innerHTML = name.value;
            id.value = dict["id"];
        }
        // I actually don't know why sharing wouldn't be in dict, but apparently on production it isn't. httpd_build_query
        // must act differently :(

        if ("sharing" in dict)
            sharing = dict["sharing"];
        else
            sharing = "";

        // console.debug("sharing is: " + sharing);

        if (sharedHash == "" && modNum == "") {     // only write shared stuff if we are not displaying a shared model
            if (sharing == '' || sharing == 0) {
                writeWannaShareLink(modelID);
            } else
                createSharingLink(modelID, sharing);
        }

        for (i=0; i <numConverts; i++) {
            type = dict["CONV"+i+"convType"];

            addRowConvertTable(type, modNum);

            yourInv = document.getElementById("CONV"+i+"convAmount"+modNum);
            totalInv = document.getElementById("CONV"+i+"convTotal"+modNum);
            discount = document.getElementById("CONV"+i+"discount"+modNum);
            cap = document.getElementById("CONV"+i+"cap"+modNum);
            custom = document.getElementById("CONV"+i+"custom"+modNum);
            preexist = document.getElementById("CONV"+i+"preexist"+modNum);
            shares = document.getElementById("CONV"+i+"shares"+modNum);


            yourInv.value = $.number(dict["CONV"+i+"convAmount"]);
            if (type != "YCVC SAFE")
                totalInv.value = $.number(dict["CONV"+i+"convTotal"]);
            discount.value = $.number(dict["CONV"+i+"discount"]);
            if (type != "YCVC SAFE")
                cap.value = $.number(dict["CONV"+i+"cap"]);
            custom.value = dict["CONV"+i+"custom"];
            preexist.value = dict["CONV"+i+"preexist"];
            shares.value = $.number(dict["CONV"+i+"shares"]);

        }
        if (numFounders > 0)
            $("#founderTable"+modNum).css("display", "block");
        if (numOtherCommon > 0)
            $("#otherCommonTable"+modNum).css("display", "block");
        if (numConverts > 0)
            $("#convertTable"+modNum).css("display", "block");
        $("#equityTable").css("display", "block");

    });

}   // loadmodelbyid()

/*
 * load a model for a guest to play with
 */
function loadSampleModel(mod) {

    console.debug("loadSampleModel. mod: " + mod);

    var yourInv;
    var totalInv;
    var discount;
    var cap;
    var custom;
    var preexist;
    var type;

    var vcInv = document.getElementById("investAmount"+mod);
    var angelInv = document.getElementById("yourAmount"+mod);
    var val = document.getElementById("valuation"+mod);
    var valispre = document.getElementById("valispre"+mod);
    //ar preopt = document.getElementById("opoolPre"+mod);
    var preOptionPoolSize = document.getElementById("optionPoolSizeForm"+mod);
    var preGranted = document.getElementById("grantedOptionsForm"+mod);
    var prePromised = document.getElementById("promisedOptionsForm"+mod);
    var preUnissued = document.getElementById("unissuedOptionsForm"+mod);
    var postopt = document.getElementById("opoolPost"+mod);
    var shares = document.getElementById("sharesOut"+mod);


    vcInv.value         = $.number(2000000);
    angelInv.value      = $.number(0);
    val.value           = $.number(8000000);
    valispre.checked    = true;
    postopt.value       = 10;
    // preopt.value        = $.number(300000);
    // shares.value        = $.number(10000000);
    //$('#optionPoolSizeForm'+mod).val($.number(500000))
    preOptionPoolSize.value = 500000;
    preGranted.value = 250000;
    prePromised.value = 100000;
    preUnissued.value = 150000;

    addRowConvertTable("YCVC SAFE", mod);
    addRowConvertTable("SAFE", mod);
    addRowCommonTable("YC", "YC", 1000000, mod);
    addRowCommonTable("Founder", "Joe Founder", 4000000, mod);
    addRowCommonTable("Founder", "Jane Founder", 5000000, mod);

    $("#equityTable"+mod).css("display", "block");
    $("#founderTable"+mod).css("display", "block");
    $("#otherCommonTable"+mod).css("display", "block");
    $("#optionsTable").css("display", "block");

    yourInv = document.getElementById("CONV1convAmount"+mod);
    totalInv = document.getElementById("CONV1convTotal"+mod);
    discount = document.getElementById("CONV1discount"+mod);
    cap = document.getElementById("CONV1cap"+mod);
    custom = document.getElementById("CONV1custom"+mod);
    preexist = document.getElementById("CONV1preexist"+mod);
    shares = document.getElementById("CONV1shares"+mod);


    yourInv.value       = $.number(50000);
    totalInv.value      = $.number(1000000);
    discount.value      = $.number(10);
    cap.value           = $.number(6000000);

}

function loadCompareModel(selected, mod) {
    /*
     * grab the selected model id
     * send it to investment with a request for data
     * grab the data and put it into the forms (and potentially add new convert rows)
     */
    console.debug("loadCompareModel: selected value is: " + selected.value + " mod is: " + mod);
    clearModel(mod);
    if (selected.value != 0) {
        /*
         * this is a request for a model
         */
        if (selected.value == -1) {
            loadSampleModel(mod);
        } else {
            loadModelByID(selected.value, mod, "");
            $("#convertTable"+mod).css("display","none");
            $("#equityTable"+mod).css("display","inline");
            $("#outputDiv").css("display","inline");

        }

    }


}

function clearModel(mod) {
    /*
     * clear the model
     */


    var name = document.getElementById('modelName'+mod);
    var id = document.getElementById('modelID'+mod);
    var count = document.getElementById("convertCount"+mod);
    var c = count.value;    // number of converts

    /* if (c > 0) {
        var yourInv = document.getElementById("CONV0convAmount"+mod);
        var totalInv = document.getElementById("CONV0convTotal"+mod);
        var discount = document.getElementById("CONV0discount"+mod);
        var cap = document.getElementById("CONV0cap"+mod);
    }*/

    var vcInv = document.getElementById("investAmount"+mod);
    var val = document.getElementById("valuation"+mod);
    var valispre = document.getElementById("valispre"+mod);
    // var preopt = document.getElementById("opoolPre"+mod);
    var postopt = document.getElementById("opoolPost"+mod);
    // var shares = document.getElementById("sharesOut"+mod);


    console.debug("clearmodel: c= " + c);
    if (c == 0)
        return;


    /*  I'm pretty sure this was relevant only in the past when CONV0 was always there. Its not now. Kill this code
    once you are sure.

    yourInv.value = 0;
    totalInv.value = 0;
    discount.value = 0;
    cap.value = 0;
    */

    vcInv.value = 0;
    val.value = 0;
    valispre.checked = true;
    // preopt.value = 0;
    postopt.value = 0;
    // shares.value = 0;

    name.value = "";
    id.value = 0;
    count.value = 0;

    var table = document.getElementById("convertTable"+mod);

    for (i=0; i < c; i++) {
        table.deleteRow(-1);  // converts are at the end
    }

    $("#founderTable tbody tr").remove();
    $("#founderCount"+mod).val(0);

    $("#otherCommonTable tbody tr").remove();
    $("#otherCommonCount"+mod).val(0);

    /*
     * also - hide any output
     */
    $("#convertTable").css("display", "none");
    $("#optionsTable").css("display", "none");
    $("#equityTable").css("display", "none");
    $("#founderTable").css("display", "none");
    $("#otherCommonTable").css("display", "none");
    $("#outputDiv").css("display", "none");
    $("#financingSummary").css("display","none");
    $("#financingDetails").css("display","none");

    /*
     * set option values to 0
     */
    $("#optionPoolSizeForm").val(0);
    $("#grantedOptionsForm").val(0);
    $("#promisedOptionsForm").val(0);
    $("#unissuedOptionsForm").val(0);


}

function createSharingLink(id, sharing) {

    url = "http://angelcalc.com/model?mod=" + id + "&dispShare=";

    if (sharing) {
        url += sharing;
        writeSharingLink(url, id);
    } else {

        $.get("model?shareModel="+id, function(data,status) {
            var dict = parseQueryStringToDictionary(data);

            url += dict["hash"];

            writeSharingLink(url, id);


        });
    }
}

function turnOffSharing(id) {


    $.get("model?unShareModel="+id, function(data,status) {
        var dict = parseQueryStringToDictionary(data);
        console.debug("unshare:" + data);

        if ("status" in dict) {
            var result = dict["status"];

            if (result == "ok")
                writeWannaShareLink(id);
            else
                angelJSerror("Couldn't turn off sharing! Returned: '" + result + "'");
        } else
                angelJSerror("Couldn't turn off sharing(2)! Returned: " + data);
    });

}

function writeWannaShareLink(id) {

    var share = document.getElementById('modelShareDiv');
    var params = id + "," + "''";
    share.innerHTML = '<a href="javascript:onclick=createSharingLink(' + params + ')"' + '>Share This Model</a>';
    share.style.display = "inline";

}

function writeSharingLink(url, id) {

    var share = document.getElementById('modelShareDiv');

    share.innerHTML = "<div style='float:right; text-align: right; width:1000px; border: 0px solid red;'>" +
        "<table><tr><td>Copy this URL to share this model:</td> " +
        '<td><input type=text size="50" id="shareID" value="' + url + '">' +
        '<button class="clipButton" id="clipButton" data-clipboard-target="#shareID">' +
        '<img src="img/clippy.svg" width="13" alt="Copy to clipboard" title="Copy to clipboard">' +
        '</button></td><td style="padding:10px"><a href="javascript:onclick=turnOffSharing(' + id + ')">Stop Sharing</a></td></tr></div>';

    var button = document.getElementById('clipButton');
    var clipboard = new Clipboard(button);

    share.style.display = "inline";
}

function clearSharingLink() {
    var share = document.getElementById('modelShareDiv');

    share.innerHTML = "";
}

/* Parse QueryString using String Splitting */
function parseQueryStringToDictionary(queryString) {
    var dictionary = {};
    // console.debug("parseQuery: queryString: " + queryString);

    // remove the '?' from the beginning of the querystring
    // if it exists
    if (queryString.indexOf('?') === 0) {
        queryString = queryString.substr(1);
    }
    // Step 0: determine if an error and return notify / return null if so
    if (queryString.indexOf('AC-ERR') === 0) {
        var keyValuePair = queryString.split('=');
        err = keyValuePair[1];
        angelJSerror(err);
        return(null);
    }

    // Step 1: separate out each key/value pair
    var parts = queryString.split('&');

    for(var i = 0; i < parts.length; i++) {
        var p = parts[i];
        // Step 2: Split Key/Value pair
        var keyValuePair = p.split('=');

        // Step 3: Add Key/Value pair to Dictionary object
        var key = keyValuePair[0];
        var value = keyValuePair[1];

        // decode URI encoded string
        value = decodeURIComponent(value);
        value = value.replace(/\+/g, ' ');

        dictionary[key] = value;
    }

    // Step 4: Return Dictionary Object
    return dictionary;
}

function toggleStartPageAndTables() {
    startDiv = document.getElementById('angelStartpageContainer');
    tablesDiv = document.getElementById('angelTablesContainer');
    if (startDiv.style.display == "none") {
        startDiv.style.display = "block";
        tablesDiv.style.display = "none";
    } else {
        startDiv.style.display = "none";
        tablesDiv.style.display = "block";
    }
}

function showStartPage() {
    startDiv = document.getElementById('angelStartpageContainer');
    tablesDiv = document.getElementById('angelTablesContainer');
        startDiv.style.display = "block";
        tablesDiv.style.display = "none";
}

function showTables() {
    startDiv = document.getElementById('angelStartpageContainer');
    tablesDiv = document.getElementById('angelTablesContainer');
    startDiv.style.display = "none";
    tablesDiv.style.display = "block";
}

function toggleInstructions() {
    insDiv = document.getElementById('modelInstructions');
    action = document.getElementById('showHideInstructions');
    if (action.innerHTML == "hide") {
        insDiv.style.display = "none";
        action.innerHTML = "show";
    } else {
        insDiv.style.display = "block";
        action.innerHTML = "hide";
    }
}

function toggleX(x) {
    console.debug("toggleX: x: " + x);
    div = document.getElementById(x);
    action = document.getElementById(x+"Action");
    if (action.innerHTML == "Hide") {
        div.style.display = "none";
        action.innerHTML = "Show";
    } else {
        div.style.display = "block";
        action.innerHTML = "Hide";
    }

}


/*
 * Import / Export / Tax
 *
 */
function importChooser(element, choice) {
    $("#uploadType").val(choice);       // set upload type
    //console.debug(element);
    /*
     * first reset links
     */
    $("#importInvestments").css("color","#337ab7");
    $("#importPayouts").css("color","#337ab7");
    $("#importYCFile").css("color","#337ab7");

    /*
     * make selected link black and display correct format
     */
    $(element).css("color","black");
    if (choice == "investments") {
        $("#importPayoutsFormat").css("display","none");
        $("#importYCFileFormat").css("display","none");
        $("#importInvestmentsFormat").css("display","block");
        $('#uploadSkipDiv').css("display", "block");
    } else if (choice == "payouts") {
        $("#importPayoutsFormat").css("display","block");
        $("#importInvestmentsFormat").css("display","none");
        $("#importYCFileFormat").css("display","none");
        $('#uploadSkipDiv').css("display", "none");
    } else {    // YC file
        $("#importPayoutsFormat").css("display","none");
        $("#importInvestmentsFormat").css("display","none");
        $("#importYCFileFormat").css("display","block");
    }


}   // importchooser

function importFundChooser(element, choice) {
    $("#uploadType").val(choice);       // set upload type
    //console.debug(element);
    /*
     * first reset links
     */
    $("#importFundInvestments").css("color","#337ab7");
    $("#importFundPayouts").css("color","#337ab7");

    /*
     * make selected link black and display correct format
     */
    $(element).css("color","black");
    if (choice == "fundinvestments") {
        $("#importFundPayoutsFormat").css("display","none");
        $("#importFundInvestmentsFormat").css("display","block");
        $('#uploadSkipDiv').css("display", "block");
    } else if (choice == "fundpayouts") {
        $("#importFundPayoutsFormat").css("display","block");
        $("#importFundInvestmentsFormat").css("display","none");
        $('#uploadSkipDiv').css("display", "none");
    }

}       // importfundchooser

function exportManager() {
    console.debug("exportManager");

    $("#dialogExport").dialog({
        resizable: false,
        height: 400,
        width: 400,
        modal: true,
        buttons: {
            Proceed: function() {
                proceed();
                $(this).dialog("close");
                $.notify("Export Completed.", "success");
            },
            Cancel: function() {
                $(this).dialog("close");

            }
        }
    });

    function proceed(){

        var exp = $('input[name=exportType]:checked').val();

        if (exp == "investments")
            exportInvestments();
        else if (exp == "payouts")
            exportPayouts();

    }

}       // exportManager

function exportInvestments() {
    var headers = ["date","company","amount","round","notes"];
    var csv = '';

    /*
     create the header line.
     */
    for (var i=0;i<headers.length; i++) {
        if (i != 0)
            csv += ",";     // the comma separator!
        csv += headers[i];
    }
    csv += "\n";            // end the header line
    var inv = Investments.list[0];
    //console.debug(Object.keys(inv));
    /*
     * now all the investments
     */
    for (i=0; i<Investments.list.length; i++) {
        inv = Investments.list[i];
        csv += inv.inv_date;
        csv += "," + Companies.company(inv.company_id).name;
        csv += "," + inv.invest_amount;
        csv += "," + inv.round;
        csv += "," + inv.notes;
        csv += "\n";
    }

    var pom = document.createElement('a');
    pom.setAttribute('href', 'data:text/csv;charset=utf-8,' + encodeURIComponent(csv));
    pom.setAttribute('download', 'investments.csv');
    pom.click();

}   // export investments

function exportPayouts() {
    var headers = ["date","company","type","cash", "shares", "share price","symbol","shr type","shr loc","notes"];
    var csv = '';

    /*
     create the header line.
     */
    for (var i=0;i<headers.length; i++) {
        if (i != 0)
            csv += ",";     // the comma separator!
        csv += headers[i];
    }
    csv += "\n";            // end the header line
    var po = Payouts.list[0];
    // console.debug(Object.keys(po));
    /*
     * now all the investments
     */
    for (i=0; i<Payouts.list.length; i++) {
        po = Payouts.list[i];
        csv += po.payDate;
        csv += "," + Companies.company(po.companyID).name;
        csv += "," + po.type;
        csv += "," + po.cash;
        csv += "," + po.shares;
        csv += "," + po.sharePrice;
        csv += "," + po.symbol;
        csv += "," + po.shareType;
        csv += "," + po.shareLoc;
        if (po.notes != null)
            csv += "," + po.notes;
        else
            csv += ",";
        csv += "\n";
    }

    var pom = document.createElement('a');
    pom.setAttribute('href', 'data:text/csv;charset=utf-8,' + encodeURIComponent(csv));
    pom.setAttribute('download', 'distributions.csv');
    pom.click();
}
function taxManager() {
    console.debug("taxManager");

    $("#dialogTax").dialog({
        resizable: false,
        height: 680,
        width: 500,
        modal: true,
        buttons: {
            Proceed: function() {
                proceed();
                $(this).dialog("close");
                //$.notify("Tax Report Completed.", "success");
            },
            Cancel: function() {
                $(this).dialog("close");

            }
        }
    });

    function proceed(){

        var taxPeriod = $('select[name=taxPeriod]').val();
        // console.debug("taxPeriod: " + taxPeriod);
        if (taxPeriod != null && taxPeriod != "") {
            taxReport(taxPeriod);
            $.notify("Tax Report Completed for: " + taxPeriod, "success");
        }

    }

}       // taxManager

function taxReport(period) {
    var headers = ["entry type","date received","date purchased","invest amount", "total invest","basis used/allocated",
                    "company name","prev names","exit date","QSB?", "dist type","cash rcvd", "shares rcvd", "share price","symbol","last dist?"];
    var quarters = ["Q1","Q2","Q3","Q4"];
    var quarterStart = ["1/1/","4/1/","7/1/","10/1/"];
    var quarterEnd = ["3/31/","6/31/","9/31/","12/31/"];
    var csv = '';
    var invests = [];
    var companies = [];
    var year;
    var startTime = 0, endTime = 0;
    var co;

    var deadCos = $('#taxDeadCos').is(":checked");

    quarterIndex = quarters.indexOf(period);
    console.debug("(quarterIndex, period): " + angelParens(quarterIndex, period));
    if (quarterIndex == -1) {
        year = period;
    } else {
        d = new Date();
        year = d.getUTCFullYear();     // year is this year
        // year = "2016";                 // just for testing
        startTime = Date.parse(quarterStart[quarterIndex]+year);
        endTime = Date.parse(quarterEnd[quarterIndex]+year);
        console.debug("(start,end): ", angelParens(quarterStart[quarterIndex]+year,quarterEnd[quarterIndex]+year));
        console.debug("(startTime,endTime): ", angelParens(startTime, endTime));
    }

    /*
     create the header line.
     */
    for (var i=0;i<headers.length; i++) {
        if (i != 0)
            csv += ",";     // the comma separator!
        csv += headers[i];
    }
    csv += "\n";            // end the header line

    for (i=0; i<Payouts.list.length; i++) {
        po = Payouts.list[i];
        poYear = new Date(po.payDate);
        if (poYear.getUTCFullYear() != year)
            continue;
        if (startTime != 0) {
            /*
             * this is a quarterly report. skip payouts outside of the quarter
             */
            poTime = poYear.getTime();
            if (poTime < startTime || poTime > endTime)
                continue;

        }

        if (companies.indexOf(po.companyID) == -1) {        // only do each company once
            companies.push(po.companyID);
            co = Companies.company(po.companyID);
            invests = Companies.investments(po.companyID);
            if (invests.length == 0)
                angelJSerror("No investment for payout with id: " + po.id);

            for (j=0; j<invests.length; j++) {
                inv = invests[j];
                csv += "Investment";
                csv += ",," + inv.inv_date;
                csv += "," + inv.invest_amount;
                csv += "," + Companies.invested(po.companyID);
                csv += "," + basisUsed(inv);
                csv += "," + co.name;
                csv += "," + co.other_names;
                csv += "," + co.exitDate;
                csv += "," + ((co.qsb == 1) ? "Y" : "N");
                csv += "\n";            // end this investment line
            }
        }

    }




    var po = Payouts.list[0];
    // console.debug(Object.keys(po));
    /*
     * now all the investments
     */
    for (i=0; i<Payouts.list.length; i++) {
        po = Payouts.list[i];
        co = Companies.company(po.companyID);
        poYear = new Date(po.payDate);
        // console.debug("taxReport: poYear / year: " + poYear.getUTCFullYear() + "/" + year)
        if (poYear.getUTCFullYear() != year)
            continue;
        if (startTime != 0) {
            /*
             * this is a quarterly report. skip payouts outside of the quarter
             */
            poTime = poYear.getTime();
            if (poTime < startTime || poTime > endTime)
                continue;

        }
        csv += "Distribution,";
        csv += po.payDate;

        csv += ",,," + Companies.invested(po.companyID);
        csv += "," + basisAllocated(po);
        csv += "," + co.name;
        csv += "," + co.other_names;
        csv += ",,," + po.type;
        csv += "," + po.cash;
        csv += "," + po.shares;
        csv += "," + po.sharePrice;
        csv += "," + po.symbol;
        csv += "," + ((po.lastPayout == 1) ? "Y" : "N");
        csv += "\n";                    // end this distribution line
    }

    /*
     * If requested, list all the no-longer active companies
     */
    if (deadCos) {
        csv += "\nInactive Companies\n";
        csv += "Name, Total Invested, Total Returned, Exit Date, Loss?\n";

        var invested, returned;
        for (i=0; i<Companies.list.length; i++) {
            if (Companies.list[i].status != "active") {
                co = Companies.list[i];
                invested = Number(Companies.invested(co.id));
                returned = Number(Companies.returned(co.id, Payouts));
                csv += co.name + ",";
                csv += invested + "," + returned;
                csv += ",";

                if (co.exitDate != "0000-00-00")
                    csv += co.exitDate;

                if (returned < invested)
                    csv += ",Y";

                csv += "\n";
            }
        }

    }

    var pom = document.createElement('a');
    pom.setAttribute('href', 'data:text/csv;charset=utf-8,' + encodeURIComponent(csv));
    filename = "tax" + year;
    if (quarterIndex != -1)
        filename += period;
    filename += ".csv";
    pom.setAttribute('download', filename);
    pom.click();

}   // taxReport

function fundsTaxManager() {
    console.debug("fundsTaxManager");

    $("#dialogFundsTax").dialog({
        resizable: false,
        height: 680,
        width: 500,
        modal: true,
        buttons: {
            Proceed: function() {
                proceed();
                $(this).dialog("close");
                //$.notify("Tax Report Completed.", "success");
            },
            Cancel: function() {
                $(this).dialog("close");

            }
        }
    });

    function proceed(){

        var taxPeriod = $('select[name=fundsTaxPeriod]').val();
        console.debug("taxPeriod: " + taxPeriod);
        if (taxPeriod != null && taxPeriod != "") {
            fundsTaxReport(taxPeriod);
            $.notify("Funds tax Report Completed for: " + taxPeriod, "success");
        }

    }

}       // fundsTaxManager

function fundsTaxReport(period) {
    var headers = ["entry type","date received","date purchased","invest amount", "total invest","basis used/allocated",
        "fund name","dist type","cash rcvd", "shares rcvd", "share price","symbol","last dist?"];
    var quarters = ["Q1","Q2","Q3","Q4"];
    var quarterStart = ["1/1/","4/1/","7/1/","10/1/"];
    var quarterEnd = ["3/31/","6/31/","9/31/","12/31/"];
    var csv = '';
    var invests = [];
    var funds = [];
    var year;
    var startTime = 0, endTime = 0;
    var fund;

    quarterIndex = quarters.indexOf(period);
    console.debug("(quarterIndex, period): " + angelParens(quarterIndex, period));
    if (quarterIndex == -1) {
        year = period;
    } else {
        var d = new Date();
        year = d.getUTCFullYear();     // year is this year
        // year = "2016";                 // just for testing
        startTime = Date.parse(quarterStart[quarterIndex]+year);
        endTime = Date.parse(quarterEnd[quarterIndex]+year);
        console.debug("(start,end): ", angelParens(quarterStart[quarterIndex]+year,quarterEnd[quarterIndex]+year));
        console.debug("(startTime,endTime): ", angelParens(startTime, endTime));
    }

    /*
     create the header line.
     */
    for (var i=0;i<headers.length; i++) {
        if (i != 0)
            csv += ",";     // the comma separator!
        csv += headers[i];
    }
    csv += "\n";            // end the header line

    for (i=0; i<FundInvestments.list.length; i++) {
        inv = FundInvestments.list[i];
        if (inv.type != "invest")
            continue;
        fund = Funds.fund(inv.fundID);
        invYear = new Date(inv.invDate);
        if (invYear.getUTCFullYear() != year)
            continue;
        if (startTime != 0) {
            /*
             * this is a quarterly report. skip payouts outside of the quarter
             */
            invTime = invYear.getTime();
            if (invTime < startTime || invTime > endTime)
                continue;

        }

        csv += "Investment";
        csv += ",," + inv.invDate;
        csv += "," + inv.investAmount;
        csv += "," + Funds.paidInCapital(inv.fundID);
        csv += "," + basisUsed(inv);
        csv += "," + fund.name;
        csv += "\n";            // end this investment line

    }


    /*
     * now all the investments
     */
    for (i=0; i<FundPayouts.list.length; i++) {
        po = FundPayouts.list[i];
        fund = Funds.fund(po.companyID);
        poYear = new Date(po.payDate);
        // console.debug("taxReport: poYear / year: " + poYear.getUTCFullYear() + "/" + year)
        if (poYear.getUTCFullYear() != year)
            continue;
        if (startTime != 0) {
            /*
             * this is a quarterly report. skip payouts outside of the quarter
             */
            poTime = poYear.getTime();
            if (poTime < startTime || poTime > endTime)
                continue;

        }
        csv += "Distribution,";
        csv += po.payDate;

        csv += ",,," + Funds.paidInCapital(po.companyID);
        csv += "," + basisAllocated(po);
        csv += "," + fund.name;
        csv += "," + po.type;
        csv += "," + po.cash;
        csv += "," + po.shares;
        csv += "," + po.sharePrice;
        csv += "," + po.symbol;
        csv += "," + ((po.lastPayout == 1) ? "Y" : "N");
        csv += "\n";                    // end this distribution line
    }

    var pom = document.createElement('a');
    pom.setAttribute('href', 'data:text/csv;charset=utf-8,' + encodeURIComponent(csv));
    filename = "tax" + year;
    if (quarterIndex != -1)
        filename += period;
    filename += ".csv";
    pom.setAttribute('download', filename);
    pom.click();

}   // fundTaxReport
/*****************************************************
 *****************************************************
 *
 *  Investments
 */
/*****************************************
 * This function gets called one when the document loads.
 * @param investorID
 */

var TimeStart;

function initializeInvestments(investorID, display) {

    console.debug("calling initializeinvestments");

    var noFeedback = false;
    if (display == "noFeedback") {
        display = "";
        noFeedback = true;
    }

    var d = new Date();
    CurrentYear = d.getUTCFullYear();     // set to this year.

    Companies = new CompaniesConstructor();
    Investments = new InvestmentsConstructor();
    Payouts = new PayoutsConstructor();

    FundInvestments = new InvestmentsConstructor();
    FundInvestments.commitMap = {};                 // extend fund investments to include map of commitments
    FundInvestments.carryMap = {};                  // and map of carried interest
    FundInvestments.commit = function(fundID) {     // and function to return commitment if there is one.
        if (fundID in FundInvestments.commitMap)
            return(FundInvestments.commitMap[fundID].investAmount);
        else
            return(0);

    }

    Funds = new CompaniesConstructor();
    Funds.paidInCapital = function(fundID) {        // extend funds to count paidInCapital
        paid = 0;
        for (var i=0; i < FundInvestments.list.length; i++) {
            fi = FundInvestments.list[i];
            if (fi.fundID == fundID && fi.type != "commit")
                paid += Number(fi.investAmount);
        }
        // console.debug("funds.paidincapital: paid= " + paid);
        return(paid);
    }
    FundPayouts = new PayoutsConstructor();

    AngelView = new AngelcalcView("angel", ["investments","companies", "payouts","values"], "investments");
    FundsView = new AngelcalcView("funds", ["funds", "fundpayouts", "fundpayments", "fundperformance","fundcashflows"], "funds");


    MyCompanies = Companies;
    MyInvestments = Investments;
    MyPayouts = Payouts;


    console.debug("in initializeInvestments: display: " + display);

    //var timeStart = new Date().getTime();
    var timeStart1 = performance.now();
    TimeStart = timeStart1;             // for use in ajaxstop

    getAngelData();



    addColumnSorters(investorID, 6, "angelInvestsHeaderCol", showInvestmentsSorted);
    addColumnSorters(investorID, 9, "angelPayoutsHeaderCol", showPayoutsSorted);
    addColumnSorters(investorID, 8, "angelValuesHeaderCol", showValuesSorted);
    addColumnSorters(investorID, 6, "angelCompaniesHeaderCol", showCompanyListSorted);
    addColumnSorters(investorID, 7, "angelTrackerHeaderCol", showTrackerSorted);
    addColumnSorters(investorID, 8, "angelFundsHeaderCol", showFundsSorted);
    addColumnSorters(investorID, 9, "angelFundPayoutsHeaderCol", showFundPayoutsSorted);
    addColumnSorters(investorID, 5, "angelFundPaymentsHeaderCol", showFundPaymentsSorted);
    addColumnSorters(investorID, 9, "angelFundPerformanceHeaderCol", showFundPerformanceSorted);



    addNavLinks(investorID);


    $(document).ajaxStop(function(){
        var timeEnd = performance.now();
        var time = timeEnd - TimeStart;

        console.debug("in initializeinvestments ajaxstop. Time taken: " + Number(time/1000).toFixed(2) + " seconds");

        /*
         *  fill in objects which store yearly and overall totals for angel and fund investments / returns
         */
        AngelTotals = new TotalsConstructor();
        AngelTotals.initialize();
        AngelTotalsFiltered = new TotalsConstructor();   // may need to do this after restore below...
        AngelTotals.filter = AngelView.filter;
        AngelTotalsFiltered.initialize();

        FundTotalsInv = new FundTotalsConstructor();
        FundTotalsInv.filter = "invest";
        FundTotalsInv.initialize();
        // console.debug("FundTotalsInv: (I, R)" + angelParens(FundTotalsInv.invested, FundTotalsInv.returned));

        FundTotalsCarry = new FundTotalsConstructor();
        FundTotalsCarry.filter = "carryonly";
        FundTotalsCarry.initialize();

        // console.debug("#####new totals: angel(I,R): " + angelParens(AngelTotals.invested, AngelTotals.returned));
        // console.debug("#####new totals: funds carry(I,R): " + angelParens(FundTotalsCarry.invested, FundTotalsCarry.returned));
        // console.debug("#####new totals: funds invest(I,R): " + angelParens(FundTotalsInv.invested, FundTotalsInv.returned));
        // console.debug("FundTotalsInv: (currval, predval)" + angelParens(FundTotalsInv.currVal, FundTotalsInv.predVal));
        // console.debug("FundTotalsCarry: (currval, predval)" + angelParens(FundTotalsCarry.currVal, FundTotalsCarry.predVal));



        updateStartPage();

        /*
         * Grab our stored view from our cookies (or store a cookie if not). If we haven't been called with
         * a requested view or this is a new login, the stored view is it.
         */

        if (display == "login") {
            display = AngelView.display;
            CurrentView.topNav = "angel";
            /*
             * this is inelegant, but the following calls force the sorting cookies to be in good shape
             */
            AngelView.saveSort("investments", "date","inv_date","DESC");
            AngelView.saveSort("payouts", "date","payDate","DESC");
            AngelView.saveSort("values", "string","name","ASC");

            FundsView.saveSort("funds", "fund","id","ASC");
            FundsView.saveSort("fundpayouts", "date","payDate","DESC");
            FundsView.saveSort("fundpayments", "date","invDate","DESC");
            FundsView.saveSort("fundperformance", "fund","id","ASC");

            /*
             * reset all cookies and leave angelview last since it will be the default
             */
            FundsView.save("all");
            AngelView.save("all");
            showStartPage();    // should integrate into AngelView!!
            chooseNav(CurrentView.topNav);

        } else if (display == "summary" || display == "") {
            showStartPage();
            chooseNav(CurrentView.topNav);
        } else {
            CurrentView.restore();
            chooseNav(CurrentView.topNav);
            // console.debug("initializeinvestments: CurrentView.view: " + JSON.stringify(CurrentView.view));
            display = CurrentView.view.display;
            showTables();
            // showStartPage();
            // console.debug("here!!!");
        }
        // display = initCookies(display);

    });
    /*
     *  jquery tooltips
     */
    $( function() {
        $( document ).tooltip({
            position: {
                my: "center bottom-10",
                at: "center top",
                using: function( position, feedback ) {
                    $( this ).css( position );
                    $( "<div>" )
                        .addClass( "arrow" )
                        .addClass( feedback.vertical )
                        .addClass( feedback.horizontal )
                        .appendTo( this );
                }
            }
        });
    } );

    if (!noFeedback)
        initFeedback("right-bottom", investorID);




}   // initializeInvestments()

function initializeCashflow(investorID) {

    console.debug("calling initializeCashflow");

    var noFeedback = false;
    var display = "";

    var d = new Date();
    CurrentYear = d.getUTCFullYear();     // set to this year.

    Companies = new CompaniesConstructor();
    Investments = new InvestmentsConstructor();
    Payouts = new PayoutsConstructor();

    FundInvestments = new InvestmentsConstructor();
    FundInvestments.commitMap = {};                 // extend fund investments to include map of commitments
    FundInvestments.carryMap = {};                  // and map of carried interest
    FundInvestments.commit = function(fundID) {     // and function to return commitment if there is one.
        if (fundID in FundInvestments.commitMap)
            return(FundInvestments.commitMap[fundID].investAmount);
        else
            return(0);

    }

    Funds = new CompaniesConstructor();
    Funds.paidInCapital = function(fundID) {        // extend funds to count paidInCapital
        paid = 0;
        for (var i=0; i < FundInvestments.list.length; i++) {
            fi = FundInvestments.list[i];
            if (fi.fundID == fundID && fi.type != "commit")
                paid += Number(fi.investAmount);
        }
        // console.debug("funds.paidincapital: paid= " + paid);
        return(paid);
    }
    FundPayouts = new PayoutsConstructor();

    AngelView = new AngelcalcView("angel", ["investments","companies", "payouts","values"], "investments");
    FundsView = new AngelcalcView("funds", ["funds", "fundpayouts", "fundpayments", "fundperformance","fundcashflows"], "funds");


    MyCompanies = Companies;
    MyInvestments = Investments;
    MyPayouts = Payouts;


    console.debug("in initializeCashflow: display: " + display);

    //var timeStart = new Date().getTime();
    var timeStart1 = performance.now();
    TimeStart = timeStart1;             // for use in ajaxstop

    getAngelData();


    $(document).ajaxStop(function(){
        var timeEnd = performance.now();
        var time = timeEnd - TimeStart;

        console.debug("in initializeinvestments ajaxstop. Time taken: " + Number(time/1000).toFixed(2) + " seconds");

        /*
         *  fill in objects which store yearly and overall totals for angel and fund investments / returns
         */
        AngelTotals = new TotalsConstructor();
        AngelTotals.initialize();
        AngelTotalsFiltered = new TotalsConstructor();   // may need to do this after restore below...
        AngelTotals.filter = AngelView.filter;
        AngelTotalsFiltered.initialize();

        FundTotalsInv = new FundTotalsConstructor();
        FundTotalsInv.filter = "invest";
        FundTotalsInv.initialize();
        // console.debug("FundTotalsInv: (I, R)" + angelParens(FundTotalsInv.invested, FundTotalsInv.returned));

        FundTotalsCarry = new FundTotalsConstructor();
        FundTotalsCarry.filter = "carryonly";
        FundTotalsCarry.initialize();

        // console.debug("#####new totals: angel(I,R): " + angelParens(AngelTotals.invested, AngelTotals.returned));
        // console.debug("#####new totals: funds carry(I,R): " + angelParens(FundTotalsCarry.invested, FundTotalsCarry.returned));
        // console.debug("#####new totals: funds invest(I,R): " + angelParens(FundTotalsInv.invested, FundTotalsInv.returned));
        // console.debug("FundTotalsInv: (currval, predval)" + angelParens(FundTotalsInv.currVal, FundTotalsInv.predVal));
        // console.debug("FundTotalsCarry: (currval, predval)" + angelParens(FundTotalsCarry.currVal, FundTotalsCarry.predVal));



        updateCashflowPage();


    });
    /*
     *  jquery tooltips
     */
    $( function() {
        $( document ).tooltip({
            position: {
                my: "center bottom-10",
                at: "center top",
                using: function( position, feedback ) {
                    $( this ).css( position );
                    $( "<div>" )
                        .addClass( "arrow" )
                        .addClass( feedback.vertical )
                        .addClass( feedback.horizontal )
                        .appendTo( this );
                }
            }
        });
    } );

    if (!noFeedback)
        initFeedback("right-bottom", investorID);


}   // initializeCashFlow
/*
 * DEPRECATED! initialize all cookies: for now we don't initialize sorting cookies and hope that works okay.
 */
function initCookies(view) {

    var cookies = Cookies.get();
    //console.debug("initCookies: " + JSON.stringify(cookies));
    if ("angelcalcView" in cookies && view != "login") {
        if (view != "") {
            if ("angelcalcFilter" in cookies)       // just being careful, but it really oughta be there
                CompanyStatusFilter = Cookies.get("angelcalcFilter");
            return(view);
        } else {
            view = Cookies.get("angelcalcView")
            if ("angelcalcFilter" in cookies)       // just being careful, but it really oughta be there
                CompanyStatusFilter = Cookies.get("angelcalcFilter");
            console.debug("We have cookies: (angelcalcView,angelcalcFilter) " + angelParens(view, CompanyStatusFilter));
            return(view)
        }
    } else {
        /*
         * either logging in (reset cookies) or have no cookies here.
         */
        console.debug("Setting cookies: view=" +  view);
        Cookies.set("angelcalcView", "investments");
        Cookies.set("angelcalcFilter", "all");
        return("");
    }

}

function updateStartPage() {


    showFundPerformanceSorted(InvestorID, 0);        /* this sets fund performance vals */
    showFundCashflows(InvestorID);                   /* this sets the rest of the fund perf. vals. Must come after the
                                                        above which also resets the carryonly values set here */

    var totalValue = AngelTotals.currVal + FundTotalsCarry.currVal + FundTotalsInv.currVal;
    var totalPredValue = AngelTotals.predVal + FundTotalsCarry.predVal + FundTotalsInv.predVal;
    var roi = 0;

    var totFundsCommitted = calcTotalFundsCommitted(this.filter);
    var totFundsInvested = calcTotalFundsInvested(this.filter);
    var totCommitLeft = totFundsCommitted - totFundsInvested;


    console.debug("updateStartPage: (val, predval)" + angelParens(AngelTotals.currVal, AngelTotals.predVal));

    /*
     * The summary Stats table
     */
    $("#startNoCompanies").html(countCompanies("all"));
    $("#startNoActiveCompanies").html(countCompanies("active"));
    $("#startNoFunds").html(countFunds("all"));
    $("#startNoFundsCommit").html(countFunds("commit"));
    $("#startCommitLeft").html("$" + totCommitLeft.toLocaleString());
    $("#startNoFundsCarry").html(countFunds("carry"));

    /*
     * The net worth / value table
     */

    $("#startNetworthAngelCurrent").html("$" + AngelTotals.currVal.toLocaleString());
    $("#startNetworthAngelPredicted").html("$" + AngelTotals.predVal.toLocaleString());

    $("#startNetworthFundsCurrent").html("$" + FundTotalsInv.currVal.toLocaleString());
    $("#startNetworthFundsPredicted").html("$" + FundTotalsInv.predVal.toLocaleString());

    $("#startNetworthFundsCarryCurrent").html("$" + FundTotalsCarry.currVal.toLocaleString());
    $("#startNetworthFundsCarryPredicted").html("$" + FundTotalsCarry.predVal.toLocaleString());

    $("#startNetworthTotalCurrent").html("$" + totalValue.toLocaleString());
    $("#startNetworthTotalPredicted").html("$" + totalPredValue.toLocaleString());




    /*
     * Now fill in the returns table
     */

    /* angel */
    $("#startActualAngelInvest").html("$" + AngelTotals.invested.toLocaleString());
    $("#startValueAngelInvest").html("$" + AngelTotals.invested.toLocaleString());
    $("#startPredictedAngelInvest").html("$" + AngelTotals.invested.toLocaleString());

    $("#startActualAngelReturn").html("$" + AngelTotals.returned.toLocaleString());
    $("#startValueAngelReturn").html("$" + (AngelTotals.currVal+AngelTotals.returned).toLocaleString());
    $("#startPredictedAngelReturn").html("$" + (AngelTotals.predVal+AngelTotals.returned).toLocaleString());

    if (AngelTotals.invested == 0)
        roi = 0;
    else
        roi = ((AngelTotals.returned/AngelTotals.invested)*100).toFixed(0);
    $("#startActualAngelROI").html(roi + "%");

    if (AngelTotals.invested == 0)
        roi = 0;
    else
        roi = (((AngelTotals.currVal+AngelTotals.returned)/AngelTotals.invested)*100).toFixed(0);
    $("#startValueAngelROI").html(roi + "%");
    if (AngelTotals.invested == 0)
        roi = 0;
    else
        roi = (((AngelTotals.predVal+AngelTotals.returned)/AngelTotals.invested)*100).toFixed(0);
    $("#startPredictedAngelROI").html(roi + "%");

    /* funds */
    $("#startActualFundsInvest").html("$" + FundTotalsInv.invested.toLocaleString());
    $("#startValueFundsInvest").html("$" + FundTotalsInv.invested.toLocaleString());
    $("#startPredictedFundsInvest").html("$" + FundTotalsInv.invested.toLocaleString());

    $("#startActualFundsReturn").html("$" + FundTotalsInv.returned.toLocaleString());
    $("#startValueFundsReturn").html("$" + (FundTotalsInv.currVal+FundTotalsInv.returned).toLocaleString());
    $("#startPredictedFundsReturn").html("$" + (FundTotalsInv.predVal+FundTotalsInv.returned).toLocaleString());

    if (FundTotalsInv.invested == 0)
        roi = 0;
    else
        roi = ((FundTotalsInv.returned/FundTotalsInv.invested)*100).toFixed(0);
    $("#startActualFundsROI").html(roi + "%");
    if (FundTotalsInv.invested == 0)
        roi = 0;
    else
        roi = (((FundTotalsInv.currVal+FundTotalsInv.returned)/FundTotalsInv.invested)*100).toFixed(0);
    $("#startValueFundsROI").html(roi + "%");
    if (FundTotalsInv.invested == 0)
        roi = 0;
    else
        roi = (((FundTotalsInv.predVal+FundTotalsInv.returned)/FundTotalsInv.invested)*100).toFixed(0);
    $("#startPredictedFundsROI").html(roi + "%");

    /* total */
    var totInvested, totReturnA, totReturnV, totReturnP;

    totInvested = AngelTotals.invested + FundTotalsInv.invested;
    $("#startActualTotalInvest").html("$" + totInvested.toLocaleString());
    $("#startValueTotalInvest").html("$" + totInvested.toLocaleString());
    $("#startPredictedTotalInvest").html("$" + totInvested.toLocaleString());

    totReturnA = AngelTotals.returned + FundTotalsInv.returned;
    $("#startActualTotalReturn").html("$" + totReturnA.toLocaleString());
    totReturnV = AngelTotals.currVal + AngelTotals.returned + FundTotalsInv.returned + FundTotalsInv.currVal;
    $("#startValueTotalReturn").html("$" + totReturnV.toLocaleString());
    totReturnP = AngelTotals.predVal + AngelTotals.returned + FundTotalsInv.returned + FundTotalsInv.predVal;
    $("#startPredictedTotalReturn").html("$" + totReturnP.toLocaleString());

    if (totInvested == 0)
        roi = 0;
    else
        roi = ((totReturnA/totInvested)*100).toFixed(0);
    $("#startActualTotalROI").html(roi + "%");
    if (totInvested == 0)
        roi = 0;
    else
        roi = ((totReturnV/totInvested)*100).toFixed(0);
    $("#startValueTotalROI").html(roi + "%");
    if (totInvested == 0)
        roi = 0;
    else
        roi = ((totReturnP/totInvested)*100).toFixed(0);
    $("#startPredictedTotalROI").html(roi + "%");



    /*
     * the activity over time table
     */
    var idString = "";
    var tmp = 0, tmp1 = 0, tmp2 = 0;
    var invBYTotal = 0;
    var amtBYTotal = 0;
    var adistBYTotal = 0;
    var ccBYTotal = 0;
    var fdistBYTotal = 0;
    var fdistcBYTotal = 0;
    var ofBYTotal = 0;
    var ifBYTotal = 0;
    var netBYTotal = 0;

    for (var x=9; x >= 0; x--) {
        var y = CurrentYear - x;
        idString = "#invYM" + x;
        // console.debug("updateStartPage: idString: " + idString);
        $(idString).html(AngelTotals.invsPerYear[y].toLocaleString());
        invBYTotal += AngelTotals.invsPerYear[y];

        idString = "#amtYM" + x;
        // console.debug("updateStartPage: idString: " + idString);
        tmp = Math.round(AngelTotals.years[y].invest_amount/1000);
        $(idString).html("$"+tmp.toLocaleString());
        amtBYTotal += tmp;

        idString = "#adistYM" + x;
        // console.debug("updateStartPage: idString: " + idString);
        tmp = Math.round(AngelTotals.years[y].return_amount/1000);
        $(idString).html("$"+tmp.toLocaleString());
        adistBYTotal += tmp;

        idString = "#ccYM" + x;
        // console.debug("updateStartPage: idString: " + idString);
        tmp = Math.round(FundTotalsInv.years[y].invest_amount/1000);
        $(idString).html("$"+tmp.toLocaleString());
        ccBYTotal += tmp;

        idString = "#fdistYM" + x;
        // console.debug("updateStartPage: idString: " + idString);
        tmp = Math.round(FundTotalsInv.years[y].return_amount/1000);
        $(idString).html("$"+tmp.toLocaleString());
        fdistBYTotal += tmp;

        idString = "#fdistcYM" + x;
        // console.debug("updateStartPage: idString: " + idString);
        tmp = Math.round(FundTotalsCarry.years[y].return_amount/1000);
        $(idString).html("$"+tmp.toLocaleString());
        fdistcBYTotal += tmp;

        idString = "#ofYM" + x;
        // console.debug("updateStartPage: idString: " + idString);
        tmp1 = Math.round(AngelTotals.years[y].invest_amount/1000+FundTotalsInv.years[y].invest_amount/1000);
        $(idString).html("$"+tmp1.toLocaleString());
        ofBYTotal += tmp1;

        idString = "#ifYM" + x;
        // console.debug("updateStartPage: idString: " + idString);
        tmp2 = Math.round(AngelTotals.years[y].return_amount/1000 + FundTotalsInv.years[y].return_amount/1000 + FundTotalsCarry.years[y].return_amount/1000);
        $(idString).html("$"+tmp2.toLocaleString());
        ifBYTotal += tmp2;

        idString = "#netYM" + x;
        // console.debug("updateStartPage: idString: " + idString);
        tmp = Math.round(tmp2-tmp1);
        if (tmp < 0)
            $(idString).html("<span  style='color:red'>$"+tmp.toLocaleString() + "</span>");
        else
            $(idString).html("$"+tmp.toLocaleString());
        netBYTotal += tmp;

    }

    $("#invBYTotal").html(invBYTotal.toLocaleString());
    $("#amtBYTotal").html("$"+amtBYTotal.toLocaleString());
    $("#adistBYTotal").html("$"+adistBYTotal.toLocaleString());
    $("#ccBYTotal").html("$"+ccBYTotal.toLocaleString());
    $("#fdistBYTotal").html("$"+fdistBYTotal.toLocaleString());
    $("#fdistcBYTotal").html("$"+fdistcBYTotal.toLocaleString());
    $("#ofBYTotal").html("$"+ofBYTotal.toLocaleString());
    $("#ifBYTotal").html("$"+ifBYTotal.toLocaleString());
    $("#netBYTotal").html("$"+netBYTotal.toLocaleString());



    /*
     * and display the summary company graph too!
     */
    pieGraphInvestStatus("startPageCompanySummary");
    $("#startPageCompanySummary").css("display","block");


}       // updateStartPage()

function updateCashflowPage() {


    //showFundPerformanceSorted(InvestorID, 0);        /* this sets fund performance vals */
    /* showFundCashflows(InvestorID);                   /* this sets the rest of the fund perf. vals. Must come after the
     above which also resets the carryonly values set here */

    var totalValue = AngelTotals.currVal + FundTotalsCarry.currVal + FundTotalsInv.currVal;
    var totalPredValue = AngelTotals.predVal + FundTotalsCarry.predVal + FundTotalsInv.predVal;
    var roi = 0;

    var totFundsCommitted = calcTotalFundsCommitted(this.filter);
    var totFundsInvested = calcTotalFundsInvested(this.filter);
    var totCommitLeft = totFundsCommitted - totFundsInvested;


    console.debug("updateStartPage: (val, predval)" + angelParens(AngelTotals.currVal, AngelTotals.predVal));

    /*
     * The summary Stats table
     */
    $("#startNoCompanies").html(countCompanies("all"));
    $("#startNoActiveCompanies").html(countCompanies("active"));
    $("#startNoFunds").html(countFunds("all"));
    $("#startNoFundsCommit").html(countFunds("commit"));
    $("#startCommitLeft").html("$" + totCommitLeft.toLocaleString());
    $("#startNoFundsCarry").html(countFunds("carry"));



    /*
     * the activity over time table
     */
    var idString = "";
    var tmp = 0, tmp1 = 0, tmp2 = 0;
    var invBYTotal = 0;
    var amtBYTotal = 0;
    var adistBYTotal = 0;
    var ccBYTotal = 0;
    var fdistBYTotal = 0;
    var fdistcBYTotal = 0;
    var ofBYTotal = 0;
    var ifBYTotal = 0;
    var netBYTotal = 0;
    var yearsArr = [];

    var sliderValue = Number($("#cflowRange").val());


    console.debug("updateCashflowPage: slider: " + sliderValue);

    var startYear = CurrentYear - sliderValue + 1;
    for (var x=0; x < 10; x++) {
        yearsArr[x] =  startYear + x;
        idString = "#cashFlow" + x;
        $(idString).html(yearsArr[x]);
    }
    console.debug("updateCashflowPage: yearsArr: " + JSON.stringify(yearsArr));

    for (var x=9; x >= 0; x--) {
        //var y = CurrentYear - x;
        var y = yearsArr[9-x];

        // console.debug("updateStartPage: idString: " + idString);
        if (y <= CurrentYear) {
            idString = "#invYM" + x;
            $(idString).html(AngelTotals.invsPerYear[y].toLocaleString());
            invBYTotal += AngelTotals.invsPerYear[y];

            idString = "#amtYM" + x;
            // console.debug("updateStartPage: idString: " + idString);
            tmp = Math.round(AngelTotals.years[y].invest_amount/1000);
            $(idString).html("$"+tmp.toLocaleString());
            amtBYTotal += tmp;

            idString = "#adistYM" + x;
            // console.debug("updateStartPage: idString: " + idString);
            tmp = Math.round(AngelTotals.years[y].return_amount/1000);
            $(idString).html("$"+tmp.toLocaleString());
            adistBYTotal += tmp;
        } else {
            idString = "#invYM" + x;
            $(idString).html('0');
            idString = "#amtYM" + x;
            $(idString).html("$"+'0');
            idString = "#adistYM" + x;
            $(idString).html("$"+'0');

        }

        idString = "#ccYM" + x;
        // console.debug("updateStartPage: idString: " + idString);
        tmp = Math.round(FundTotalsInv.years[y].invest_amount/1000);
        $(idString).html("$"+tmp.toLocaleString());
        ccBYTotal += tmp;

        idString = "#fdistYM" + x;
        // console.debug("updateStartPage: idString: " + idString);
        tmp = Math.round(FundTotalsInv.years[y].return_amount/1000);
        $(idString).html("$"+tmp.toLocaleString());
        fdistBYTotal += tmp;

        idString = "#fdistcYM" + x;
        // console.debug("updateStartPage: idString: " + idString);
        tmp = Math.round(FundTotalsCarry.years[y].return_amount/1000);
        $(idString).html("$"+tmp.toLocaleString());
        fdistcBYTotal += tmp;

        idString = "#ofYM" + x;
        // console.debug("updateStartPage: idString: " + idString);
        tmp1 = Math.round(AngelTotals.years[y].invest_amount/1000+FundTotalsInv.years[y].invest_amount/1000);
        $(idString).html("$"+tmp1.toLocaleString());
        ofBYTotal += tmp1;

        idString = "#ifYM" + x;
        // console.debug("updateStartPage: idString: " + idString);
        tmp2 = Math.round(AngelTotals.years[y].return_amount/1000 + FundTotalsInv.years[y].return_amount/1000 + FundTotalsCarry.years[y].return_amount/1000);
        $(idString).html("$"+tmp2.toLocaleString());
        ifBYTotal += tmp2;

        idString = "#netYM" + x;
        // console.debug("updateStartPage: idString: " + idString);
        tmp = Math.round(tmp2-tmp1);
        if (tmp < 0)
            $(idString).html("<span  style='color:red'>$"+tmp.toLocaleString() + "</span>");
        else
            $(idString).html("$"+tmp.toLocaleString());
        netBYTotal += tmp;

    }

    $("#invBYTotal").html(invBYTotal.toLocaleString());
    $("#amtBYTotal").html("$"+amtBYTotal.toLocaleString());
    $("#adistBYTotal").html("$"+adistBYTotal.toLocaleString());
    $("#ccBYTotal").html("$"+ccBYTotal.toLocaleString());
    $("#fdistBYTotal").html("$"+fdistBYTotal.toLocaleString());
    $("#fdistcBYTotal").html("$"+fdistcBYTotal.toLocaleString());
    $("#ofBYTotal").html("$"+ofBYTotal.toLocaleString());
    $("#ifBYTotal").html("$"+ifBYTotal.toLocaleString());
    $("#netBYTotal").html("$"+netBYTotal.toLocaleString());



    /*
     * and display the summary company graph too!
     */
    pieGraphInvestStatus("startPageCompanySummary");
    $("#startPageCompanySummary").css("display","block");


}   // updateCashflowPage()

function initFeedback(pos, investorID) {
    //set up some basic options for the feedback_me plugin
    var needEmail = true;

    if (investorID != 0)
        needEmail = false;

    fm_options = {
        bootstrap: false,
        position: pos,
        show_email: true,
        email_required: needEmail,

        show_radio_button_list: true,
        radio_button_list_required: false,
        radio_button_list_title: "Was Angelcalc useful?",
        radio_button_list_labels: ["Yes", "No"],

        name_label: "Name",
        email_label: "Email",
        message_label: "Feedback",

        name_placeholder: "Your Name",
        email_placeholder: "Your Email",
        message_placeholder: "Bugs, comments, etc. If there is a problem with a specific model, please give its name",

        name_required: true,
        message_required: true,

        show_asterisk_for_required: true,

        feedback_url: "model?feedback",

        custom_params: {
            feedback: "secret",
            user_id: investorID
            // feedback_type: "clean_complex"
        },
        delayed_options: {
            delay_success_milliseconds: 3500,
            send_success : "Thanks for the feedback!"
        }
    };
    //init feedback_me plugin
    fm.init(fm_options);

}




/*
 **************************************************************
 * View updating functions.
 */

function chooseNav(nav) {

    console.debug("chooseNav: using nav:" + nav);
    if (nav == "funds") {
        $('#angelcalcNavAngel').css("display","none");
        $('#angelcalcNavFunds').css("display","block");
        $('#topNavFunds').css("backgroundColor","white");
        $('#topNavAngel').css("backgroundColor","");
        FundsView.switchDisplay(FundsView.display);

    } else if (nav == "angel") {
        $('#angelcalcNavAngel').css("display","block");
        $('#angelcalcNavFunds').css("display","none");
        $('#topNavFunds').css("backgroundColor","");
        $('#topNavAngel').css("backgroundColor","white");
        AngelView.switchDisplay(AngelView.display);
    } else {
        /*
         * ruh oh - there is nothing else!
         */
    }
    // updateRHS(nav);    // no longer needed - see switchDisplay
}

/*
 * this runction is deprecated
 */
function updateRHS(nav) {

    if (nav == "angel") {
        display = AngelView.display;

        console.debug("updateRHS: angel: " + display);
        if (display == "investments")
            showInvestments(InvestorID);
        else if (display == "payouts")
            showPayouts(InvestorID);
        else if (display == "values")
            showValues(InvestorID);
        else  if (display == "dealflow")
            showDealflow(InvestorID);
        else if (display == "companies")
            showCompanies(InvestorID);
        else if (display == "tracker")
            showTracker(InvestorID);
        else
            console.debug("UpdateRHS angel with unknown display: " + display)

    } else if (nav == "funds") {
        display = FundsView.display;

        console.debug("updateRHS: funds: " + display);
        if (display == "funds")
            showFunds(InvestorID);
        else if (display == "fundpayouts")
            showFundPayouts(InvestorID);
        else if (display == "fundpayments")
            showFundPayments(InvestorID);
        else if (display == "fundperformance")
            showFundPerformance(InvestorID);
        else if (display == "fundcashflows")
            showFundCashflows(InvestorID);
        else
            console.debug("UpdateRHS funds with unknown display: " + display)
    }
}       // updateRHS


function updateNavSummary() {
    var totReturn = calcTotalReturn(AngelView.filter);
    var totCompanies = countCompanies(AngelView.filter);
    var totInvestments = countInvestments(AngelView.filter);
    var totInvested = calcTotalInvested(AngelView.filter);
    var activeCompanies = countCompanies("active");
    var totValue = calcTotalValue();        // for now assume counts active only

    Companies.count = totCompanies;
    Investments.count = totInvestments;

    $("#summaryInvestments").html(totInvestments);
    $("#summaryInvested").html("$" + totInvested.toLocaleString());
    $("#summaryCompanies").html(totCompanies.toLocaleString());
    $("#summaryReturned").html("$" + totReturn.toLocaleString());
    $("#summaryActive").html(activeCompanies.toLocaleString());
    $("#summaryValue").html("$" + totValue.toLocaleString());
}



/*
 * deprecating this soon!
 */
function showHideView(view) {
    var companiesContainer = document.getElementById('companiesContainer');
    var payoutsContainer = document.getElementById('payoutsContainer');
    var investmentsContainer = document.getElementById('investmentsContainer');
    var valuesContainer = document.getElementById('valuesContainer');
    var dealflowContainer = document.getElementById('dealflowContainer');
    var fundsContainer = document.getElementById('fundsContainer');

    investmentsContainer.style.display = "none";
    payoutsContainer.style.display = "none";
    companiesContainer.style.display = "none";
    valuesContainer.style.display = "none";
    dealflowContainer.style.display = "none";
    fundsContainer.style.display = "none";

    if (view == "Investments")
        investmentsContainer.style.display = "block";
    else if (view == "Payouts")
        payoutsContainer.style.display = "block";
    else if (view == "Companies")
        companiesContainer.style.display = "block";
    else if (view == "Values")
        valuesContainer.style.display = "block";
    else if (view == "Dealflow")
        dealflowContainer.style.display = "block";
    else if (view == "Funds")
        fundsContainer.style.display = "block";

}

function updateFilterNavbar(filter, show) {
    var all = document.getElementById("showAll");
    var active = document.getElementById("showActive");
    var acquired = document.getElementById("showAcquired");
    var ipo = document.getElementById("showIPO");
    var dead = document.getElementById("showDead");

    var showInvests = document.getElementById("investmentsNav");
    var showPayouts = document.getElementById("payoutsNav");
    var showCompanies = document.getElementById("companiesNav");
    var showValues = document.getElementById("valuesNav");
    var showDealflow = document.getElementById("dealflowNav");

    all.style.color = "black";
    active.style.color = "black";
    acquired.style.color = "black";
    ipo.style.color = "black";
    dead.style.color = "black";

    showInvests.style.backgroundColor = "";
    showPayouts.style.backgroundColor = "";
    showCompanies.style.backgroundColor = "";
    showValues.style.backgroundColor = "";
    showDealflow.style.backgroundColor = "";

    if (show == "investments")
        showInvests.style.backgroundColor = "white";
    else if (show == "payouts")
        showPayouts.style.backgroundColor = "white";
    if (show == "companies")
        showCompanies.style.backgroundColor = "white";
    if (show == "values")
        showValues.style.backgroundColor = "white";
    if (show == "dealflow")
        showDealflow.style.backgroundColor = "white";


    if (filter == "all")
        all.style.color = "white";
    else if (filter == "active")
        active.style.color = "white";
    else if (filter == "acquired")
        acquired.style.color = "white";
    else if (filter == "ipo")
        ipo.style.color = "white";
    else if (filter == "dead")
        dead.style.color = "white";
}




var Path = "";  // only needed this when we were developing under htdocs -- should remove!

function addNavLinks(investorID) {


    var inv = document.getElementById("investmentsNav");
    var pay = document.getElementById("payoutsNav");
    var com = document.getElementById("companiesNav");
    var val = document.getElementById("valuesNav");
    var dea = document.getElementById("dealflowNav");
    var tra = document.getElementById("trackerNav");
    var fri = document.getElementById("friendNav");
    var adv = document.getElementById("advisorNav");

    var fun = document.getElementById("fundsNav");
    var funpout = document.getElementById("fundPayoutsNav");
    var funpment = document.getElementById("fundPaymentsNav");
    var funperf = document.getElementById("fundPerformanceNav");
    var funcf = document.getElementById("fundCashflowsNav");
    var funimp = document.getElementById("importFundsNav");
    var funtax = document.getElementById("taxFundsNav");

    var mod = document.getElementById("modelNav");
    var cmp = document.getElementById("compareNav");
    var imp = document.getElementById("importNav");
    var exp = document.getElementById("exportNav");
    var tax = document.getElementById("taxNav");
    var addi = document.getElementById("addInvestmentNav");
    var addp = document.getElementById("addPayoutNav");
    var addc = document.getElementById("addCompanyNav");
    var addd = document.getElementById("addDealflowNav");
    var addtr = document.getElementById("addTrackerNav");
    var addf = document.getElementById("addFriendNav");
    var addfi = document.getElementById("addFundInvestNav");
    var addfp = document.getElementById("addFundPayoutNav");
    var addfpm = document.getElementById("addFundPaymentNav");


    if (inv.addEventListener) {
            inv.addEventListener("click", function() {
                showInvestments(investorID);
                AngelView.switchDisplay("investments");
            });
    } else {
            inv.attachEvent('onclick', function() {
                showInvestments(investorID);
                AngelView.switchDisplay("investments");
            });
    }
    if (pay.addEventListener) {
        pay.addEventListener("click", function() {
            showPayouts(investorID);
            AngelView.switchDisplay("payouts");
        });
    } else {
        pay.attachEvent('onclick', function() {
            showPayouts(investorID);
            AngelView.switchDisplay("payouts");
        });
    }
    if (com.addEventListener) {
        com.addEventListener("click", function() {
            showCompanies(investorID);
            AngelView.switchDisplay("companies");
            hideGraphics();
        });
    } else {
        com.attachEvent('onclick', function() {
            showCompanies(investorID);
            AngelView.switchDisplay("companies");
        });
    }
    if (val.addEventListener) {
        val.addEventListener("click", function() {
            showValues(investorID);
            AngelView.switchDisplay("values");
        });
    } else {
        val.attachEvent('onclick', function() {
            showValues(investorID);
            AngelView.switchDisplay("values");
        });
    }
    if (dea.addEventListener) {
        dea.addEventListener("click", function() {
            showDealflow(investorID);
            AngelView.switchDisplay("dealflow");
        });
    } else {
        dea.attachEvent('onclick', function() {
            showDealflow(investorID);
            AngelView.switchDisplay("dealflow");
        });
    }
    if (tra.addEventListener) {
        tra.addEventListener("click", function() {
            showTracker(investorID);
            AngelView.switchDisplay("tracker");
            hideGraphics();
        });
    } else {
        tra.attachEvent('onclick', function() {
            showTracker(investorID);
            AngelView.switchDisplay("tracker");
            hideGraphics();
        });
    }
    if (fun.addEventListener) {
        fun.addEventListener("click", function() {
            showFunds(investorID);
            FundsView.switchDisplay("funds");
            hideGraphics();
        });
    } else {
        fun.attachEvent('onclick', function() {
            showFunds(investorID);
            FundsView.switchDisplay("funds");
            hideGraphics();
        });
    }
    if (funpout.addEventListener) {
        funpout.addEventListener("click", function() {
            showFundPayouts(investorID);
            FundsView.switchDisplay("fundpayouts");
            hideGraphics();
        });
    } else {
        funpout.attachEvent('onclick', function() {
            showFundPayouts(investorID);
            FundsView.switchDisplay("fundpayouts");
            hideGraphics();
        });
    }
    if (funpment.addEventListener) {
        funpment.addEventListener("click", function() {
            showFundPayments(investorID);
            FundsView.switchDisplay("fundpayments");
            hideGraphics();
        });
    } else {
        funpment.attachEvent('onclick', function() {
            showFundPayments(investorID);
            FundsView.switchDisplay("fundpayments");
            hideGraphics();
        });
    }
    if (funperf.addEventListener) {
        funperf.addEventListener("click", function() {
            showFundPerformance(investorID);
            FundsView.switchDisplay("fundperformance");
            hideGraphics();
        });
    } else {
        funperft.attachEvent('onclick', function() {
            showFundPerformance(investorID);
            FundsView.switchDisplay("fundperformance");
            hideGraphics();
        });
    }
    if (funcf.addEventListener) {
        funcf.addEventListener("click", function() {
            showFundCashflows(investorID);
            FundsView.switchDisplay("fundcashflows");
            hideGraphics();
        });
    } else {
        funcf.attachEvent('onclick', function() {
            showFundCashflows(investorID);
            FundsView.switchDisplay("fundcashflows");
            hideGraphics();
        });
    }

    if (fri != null) {          // may not be there while we are developing the feature!
        if (fri.addEventListener) {
            fri.addEventListener("click", function() {
                showHideFriends();
            });
        } else {
            fri.attachEvent('onclick', function() {
                showHideFriends();
            });
        }
    }
    if (adv != null) {          // may not be there while we are developing the feature!
        if (adv.addEventListener) {
            adv.addEventListener("click", function() {
                showHideAdvisors();
            });
        } else {
            adv.attachEvent('onclick', function() {
                showHideAdvisors();
            });
        }
    }
    if (mod.addEventListener) {
        mod.addEventListener("click", function() {
            window.location = Path + "/investments?model";
            return false;
        });
    } else {
        mod.attachEvent('onclick', function() {
            window.location = Path + "/investments?model";
            return false;
        });
    }
    if (cmp.addEventListener) {
        cmp.addEventListener("click", function() {
            window.location = Path + "/investments?compare";
            return false;
        });
    } else {
        cmp.attachEvent('onclick', function() {
            window.location = Path + "/investments?compare";
            return false;
        });
    }
    if (imp.addEventListener) {
        imp.addEventListener("click", function() {
            window.location = Path + "/investments?import";
            return false;
        });
    } else {
        imp.attachEvent('onclick', function() {
            window.location = Path + "/investments?import";
            return false;
        });
    }
    if (funimp.addEventListener) {
        funimp.addEventListener("click", function() {
            window.location = Path + "/investments?importFunds";
            return false;
        });
    } else {
        funimp.attachEvent('onclick', function() {
            window.location = Path + "/investments?importFunds";
            return false;
        });
    }
    if (funtax.addEventListener) {
        funtax.addEventListener("click", function() {
            fundsTaxManager();
            return false;
        });
    } else {
        funtax.attachEvent('onclick', function() {
            fundsTaxManager();
            return false;
        });
    }
    if (exp.addEventListener) {
        exp.addEventListener("click", function() {
            exportManager();
            return false;
        });
    } else {
        exp.attachEvent('onclick', function() {
            exportManager();
            return false;
        });
    }
    if (tax.addEventListener) {
        tax.addEventListener("click", function() {
            taxManager();
            //window.location = Path + "/investments?tax";
            return false;
        });
    } else {
        tax.attachEvent('onclick', function() {
            taxManager();
            //window.location = Path + "/investments?tax";
            return false;
        });
    }

    if (addi.addEventListener) {
        $("#datepickerInv").datepicker();
        addi.addEventListener("click", function() {
            hideGraphics();
            clearAddInvestmentForm();
            showAddForm("investment");

            return false;
        });
    } else {
        $("#datepickerInv").datepicker();
        addi.attachEvent('onclick', function() {
            window.location = Path + "/investments?newInvestment";
            return false;
        });
    }
    if (addp.addEventListener) {
        $("#datepickerPay").datepicker();
        addp.addEventListener("click", function() {
            hideGraphics();
            clearAddPayoutForm("");
            showAddForm("payout");

            // window.location = Path + "/investments?newPayout";
            return false;
        });
    } else {
        $("#datepickerPay").datepicker();
        addp.attachEvent('onclick', function() {
            window.location = Path + "/investments?newPayout";
            return false;
        });
    }
    if (addc.addEventListener) {
        addc.addEventListener("click", function() {
            window.location = Path + "/companyPage?newCompany";
            return false;
        });
    } else {
        addc.attachEvent('onclick', function() {
            window.location = Path + "/companyPage?newCompany";
            return false;
        });
    }
    if (addd.addEventListener) {
        addd.addEventListener("click", function() {
            window.location = Path + "/companyPage?newProspect";
            return false;
        });
    } else {
        $("#datepickerInv").datepicker();
        addd.attachEvent('onclick', function() {
            window.location = Path + "/companyPage?newProspect";
            return false;
        });
    }
    if (addtr.addEventListener) {
        $("#datepickerTra").datepicker();
        addtr.addEventListener("click", function() {
            hideGraphics();
            clearTrackerForm();
            showAddForm("tracker");

            return false;
        });
    } else {
        $("#datepickerTra").datepicker();
        addtr.attachEvent('onclick', function() {
            window.location = Path + "/investments?newTracker";
            return false;
        });
    }
    if (addf != null) {
        if (addf.addEventListener) {
            addf.addEventListener("click", function() {
                angelJSerror("Coming soon!");
                return false;
            });
        } else {
            addf.attachEvent('onclick', function() {
                angelJSerror("Coming soon!");
                return false;
            });
        }
    }
    if (addfi.addEventListener) {
        $("#datepickerFundAdd").datepicker();
        addfi.addEventListener("click", function() {
            hideGraphics();
            clearAddFundInvestmentForm();
            showAddForm("fundinvestment");

            return false;
        });
    } else {
        $("#datepickerFundAdd").datepicker();
        addfi.attachEvent('onclick', function() {
            window.location = Path + "/investments?newFundInvestment";
            return false;
        });
    }
    if (addfp.addEventListener) {
        $("#datepickerPayfund").datepicker();
        addfp.addEventListener("click", function() {
            hideGraphics();
            clearAddPayoutForm("fund");
            showAddForm("fundpayout");

            return false;
        });
    } else {
        $("#datepickerPayfund").datepicker();
        addfp.attachEvent('onclick', function() {
            window.location = Path + "/investments?newFundPayout";
            return false;
        });
    }
    if (addfpm.addEventListener) {
        $("#datepickerFundPayment").datepicker();
        addfpm.addEventListener("click", function() {
            hideGraphics();
            clearAddFundPaymentForm("fund");
            showAddForm("fundpayment");

            return false;
        });
    } else {
        $("#datepickerPaymentfund").datepicker();
        addfpm.attachEvent('onclick', function() {
            window.location = Path + "/investments?newFundPayment";
            return false;
        });
    }


}       // addNavLinks

/*
 **************************************************************
 *  End View updating functions.
 */



/*
 * show the specifed add form. if called with "", then all add forms are hidden.
 */
function showAddForm(form) {

    console.debug("showAddForm - form: " + form);

    $("#addInvestmentDiv").css("display","none");
    $("#addTrackerDiv").css("display","none");
    $("#addPayoutDiv").css("display","none");
    $("#addFundInvestDiv").css("display","none");
    $("#addFundPayoutDiv").css("display","none");
    $("#addFundPaymentDiv").css("display","none");
    $("#transparentHelpDiv").css("display","none");

    if (form != "")
         AngelView.clearDisplay();   // display nothing!

    if (form == "investment")
        $("#addInvestmentDiv").css("display","block");
    if (form == "tracker")
        $("#addTrackerDiv").css("display","block");
    else if (form == "payout")
        $("#addPayoutDiv").css("display","block");
    else if (form == "fundinvestment")
        $("#addFundInvestDiv").css("display","block");
    else if (form == "fundpayout")
        $("#addFundPayoutDiv").css("display","block");
    else if (form == "fundpayment")
        $("#addFundPaymentDiv").css("display","block");
    else if (form != "")
        console.debug("showaddform with unknown form: " + form);

}   // showaddform


function showCompanyNoteAddForm() {
    var div = document.getElementById("addCompanyNoteDiv");
    var date = new Date();
    var options = {timeZone: 'UTC'};

    $("#datepickerCompanyNote").datepicker("setDate",date.toLocaleDateString('en-US',options));

    div.style.display = "block";

}

function companyEditRequest(companyName) {
    var div = document.getElementById("editCompanyRequestDiv");

    //div.style.display = "block";

    $.notify("Your request to get edit access to '" + companyName + "' confirmed.", "success");
    /*
     * OK - do something with the request now!!!
     */
}

function personEditRequest(id, name) {


    $.notify("Your request to get edit access to '" + name + "' confirmed.", "success");
    /*
     * OK - do something with the request now!!!
     */
}

function deleteCompanyNote(companyID, noteID) {
    // var tr = $(t).closest("tr")
    console.debug("deleteCompanyNote id: " + noteID);

    if (noteID == 0) {
        console.debug("deleteCompanyNote called with 0 id");
        angelJSerror("There is a problem: this companyNote is missing. Please contact support.");
        return;
    }


    $("#dialogDeleteCompanyNote").dialog({
        resizable: false,
        height: 200,
        width: 400,
        modal: true,
        buttons: {
            Proceed: function() {
                proceed(companyID, noteID);
                $(this).dialog("close");
            },
            Cancel: function() {
                $(this).dialog("close");
            }
        }
    });

    function proceed(companyID, noteID){
        window.location.href = "/companyPage?id=" + companyID + "&deleteCompanyNote=" + noteID;
    }

}   // deleteCompanyNote

function editCompanyNote(companyID, noteID) {

    console.debug("editCompanyNote with id : " + noteID);

    var addDiv = document.getElementById('addCompanyNoteDiv');

    document.getElementById("newCompanyNoteButton").innerHTML = "Update Note";

    /*
     *
     */
    $.get("/companyPage?id="+companyID+"&ajaxNote="+noteID, function(data,status) {
        var dict = parseQueryStringToDictionary(data);
        if (dict == null)
            return;

        var date = new Date(dict["noteDate"]);
        var options = {timeZone: 'UTC'};
        var div = document.getElementById("addCompanyNoteDiv");
        var valuation = dict["valuation"];
        var shares = dict["FDshares"];
        var price = dict["sharePrice"];
        var type = dict["type"];
        var round = dict["round"];
        var invested = dict["invested"];
        var split = dict["splitRatio"];

        if (type == "valuation update") {
            $("#companyValuationDiv").css("display", "inline");
            $("#companyStockSplitDiv").css("display", "none");
            $("#companyExitPredictionDiv").css("display", "none");
            $("#companyPublic").css("display", "none");
            $("#companyPublicText").html("Note: This information will never be public.");
            $("#companyPublic").prop("checked", 0);
        } else if (type == "exit prediction") {
            $("#companyExitPredictionDiv").css("display", "inline");
            $("#companyStockSplitDiv").css("display", "none");
            $("#companyValuationDiv").css("display", "none");
            $("#companyPublic").css("display", "none");
            $("#companyPublicText").html("Note: This information will never be public.");
            $("#companyPublic").prop("checked", 0);
        } else if (type == "stock split") {
            $("#companyStockSplitDiv").css("display", "inline");
            $("#companyValuationDiv").css("display", "none");
            $("#companyExitPredictionDiv").css("display", "none");
            $("#companyPublic").css("display", "none");
            $("#companyPublicText").html("Note: This information will never be public.");
            $("#companyPublic").prop("checked", 0);
        } else {
            $("#companyValuationDiv").css("display", "none");
            $("#companyExitPredictionDiv").css("display", "none");
            $("#companyStockSplitDiv").css("display", "none");
            $("#companyPublic").css("display", "inline");
            $("#companyPublicText").html("Public? If checked, this note will be visible to other Angelcalc users");
            $("#companyValuation").val(0);
            $("#companyFDshares").val(0);
            $("#companySharePrice").val(0);
            $("#companyExitSharePrice").val(0);
        }

        $("#datepickerCompanyNote").datepicker("setDate",date.toLocaleDateString('en-US',options));
        $("#companyNoteType").val(type);
        var notes = dict["notes"];

        notes = removeNewlinesAndSlashes(notes);

        console.debug("editCompany: (valuation, shares) " + angelParens(valuation, shares));
        console.debug("editCompany: (invested, round) - roundmap " + angelParens(invested, round) + " - " + JSON.stringify(RoundMap));
        console.debug("editCompany: type " + type);
        $("#companyNote").val(notes);
        $("#companyPublic").prop("checked", Number(dict["public"]));
        $('#companyNoteAction').val("updateCompanyNote");
        $('#companyNoteUpdate').val(noteID);
        $("#companyValuation").val(valuation);
        $("#companyFDshares").val(shares);
        $("#companySharePrice").val(price);
        $("#companyExitSharePrice").val(price);
        $("#companyStockSplitRatio").val(split);
        $("#companyInvested").val(invested);
        $("#companyRound").select2("val", null);
        if (round != "" && round != null)
            $("#companyRound").select2("val", RoundMap[(round).toLowerCase()]);



        div.style.display = "block";

    });


}   // editCompanyNote

function editFundNote(fundID, noteID) {

    console.debug("editFundNote with id : " + noteID);

    var addDiv = document.getElementById('addFundNoteDiv');

    document.getElementById("newFundNoteButton").innerHTML = "Update Note";

    /*
     *
     */
    $.get("/fundPage?id="+fundID+"&ajaxFundNote="+noteID, function(data,status) {
        var dict = parseQueryStringToDictionary(data);
        if (dict == null)
            return;

        var date = new Date(dict["noteDate"]);
        var options = {timeZone: 'UTC'};
        var div = document.getElementById("addFundNoteDiv");
        var capital = dict["capitalAccount"];
        var predict = dict["predictedReturn"];
        var type = dict["type"];

        if (type == "valuation update") {
            $("#fundValuationDiv").css("display", "inline");
            $("#fundPublic").css("display", "none");
            $("#fundPublicText").html("Note: This information will never be public.");
            $("#fundPublic").prop("checked", 0);
        } else if (type == "exit prediction") {
            $("#fundExitPredictionDiv").css("display", "inline");
            $("#fundPublic").css("display", "none");
            $("#fundPublicText").html("Note: This information will never be public.");
            $("#fundPublic").prop("checked", 0);
        } else {
            $("#fundValuationDiv").css("display", "none");
            $("#fundPublic").css("display", "inline");
            $("#fundPublicText").html("Public? If checked, this note will be visible to other Angelcalc users");
            $("#fundCapitalAccount").val(0);
            $("#fundPredictedReturn").val(0);
        }

        $("#datepickerFundNote").datepicker("setDate",date.toLocaleDateString('en-US',options));
        $("#fundNoteType").val(type);
        var notes = dict["notes"];

        notes = removeNewlinesAndSlashes(notes);

        console.debug("editFundNote: (capital, predicted)" + angelParens(capital, predict));
        console.debug("editFundNote: type " + type);
        $("#fundNote").val(notes);
        $("#fundPublic").prop("checked", Number(dict["public"]));
        $('#fundNoteAction').val("updateFundNote");
        $('#fundNoteUpdate').val(noteID);
        $("#fundCapitalAccount").val(capital);
        $("#fundPredictedReturn").val(predict);
        div.style.display = "block";

    });


}   // editFundNote

function addColumnSorters(investorID, numColumns, id, func) {

    for (i=1; i < numColumns+1; i++) {
        // console.debug("addColumnSorters: " + id + i);
        var co = document.getElementById(id+i);

        // console.debug("addColumnSorters: this.id,this.mySort: " + this.id + "," + this.mySort);
        if (co.addEventListener) {
            co.addEventListener("click", function() {
                func(investorID, this)
            });
        } else {
            co.attachEvent('onclick', function() {
                func(investorID, this)
            });
        }
    }

}

function showInvestments(investorID) {

    // console.debug("showInvestments: caller is: " + arguments.callee.caller.toString());

    InvestorID = investorID;    // keep investor ID around as global

    var contentDiv = document.getElementById('content');

    showInvestmentsSorted(investorID, 0, 0);
    comboGraphInvestResults("", "summaryGraph1");
    pieGraphInvestStatus("summaryGraph2");
    // console.debug("summary: " + JSON.stringify(InvestSummary));

}

function showInvestmentsSorted(investorID, cell, friend) {

    var contentSummaryDiv = document.getElementById('contentSummary');
    var summCompanies = document.getElementById('summaryCompanies');
    var summInvested = document.getElementById('summaryInvested');
    var summReturned = document.getElementById('summaryReturned');

    var sort = "date";
    var property = "inv_date";
    var direction = "DESC";
    var inv;

    /*
     * if cell is 0, this is the first call when the page is loaded, so the default values above will apply.
     * unless cookies are set, in which case cookie values take precedence.
     */
    if (cell != 0) {
        //console.debug("showInvestmentsSorted investorID = " + investorID + " sort=" + cell.getAttribute("mySort") + " prop=" + cell.getAttribute("myProp") + " cellid=" + cell.id);
        sort = cell.getAttribute("mySort");
        property = cell.getAttribute("myProp");
        direction = sortToggle(cell);

        AngelView.saveSort("investments",sort,property, direction);

    } else {
        AngelView.restore();
        sort = AngelView.sortCol["investments"];
        property = AngelView.sortProp["investments"];
        direction = AngelView.sortDir["investments"];
        console.debug("showInvestmentsSorted: (sort,property): " + angelParens(sort, property));
    }


    clearInvestmentTable();     // remove existing sort

    Investments.list.sort(function(a,b) {

        return investmentSorter(a, b, sort, direction, property);

    });
    // console.debug("here");

    var count = 0;
    for (i=0; i<Investments.list.length; i++) {
        inv = Investments.list[i];
        if ((AngelView.filter == "all" || inv.status == AngelView.filter) && inv.status != "prospect") {
            addRowInvestmentTable(inv, count);
            count++;
        }
    }

    console.debug("showInvestmentsSorted all rows added ");

    if (Investments.list.length == 0 && !friend) {

        $("#transparentHelpDiv").css("display", "block");
        hideGraphics();

    } else {
        $("#transparentHelpDiv").css("display", "none");
    }

}



function calcTotalInvested(filter) {
    var ret = 0;
    var amount = 0;
    var inv;
    var co;
    for (i = 0; i<Investments.list.length; i++) {
        inv = Investments.list[i];
        co = Companies.company(inv.company_id);

        if (filter == "all" || co.status == filter)
            ret += Number(inv.invest_amount);

    }
    return(ret);
}

function countInvestments(filter) {
    var count = 0;
    var inv;
    var co;
    for (i = 0; i<Investments.list.length; i++) {
        inv = Investments.list[i];
        co = Companies.company(inv.company_id);

        if (filter == "all" || co.status == filter)
            count++;

    }
    return(count);
}

function calcTotalReturn(filter) {
    var ret = 0;
    var amount = 0;
    for (i = 0; i<Payouts.list.length; i++) {
        po = Payouts.list[i];
        co = Companies.company(po.companyID);
        // console.debug("calcTotalReturn: co:" + co)   // debug
        if (co != null) {               // shouldn't happen unless there's a bug
            if (po.type == "cash")
                amount = Number(po.cash);
            else
                amount = Math.round(Number(po.shares)*Number(po.sharePrice));
            if (filter == "all" || co.status == filter)
                ret += amount;
        }

    }
    return(ret);
}

function calcTotalValue() {
    var val = 0;
    var shares = 0;

    for (c in Companies.map) {
        co = Companies.map[c];
        if (co.status == "active") {
            if (co.sharePrice > 0) {
                shares = Companies.shares(co.id);
                val += shares * co.sharePrice;
            }
        }
    }
    console.debug("calcTotalValue: "+ Math.round(val));
    return(Math.round(val));

}

function calcPredictedValue() {
    var val = 0;
    var shares = 0;

    for (c in Companies.map) {
        co = Companies.map[c];
        if (co.status == "active") {
            if (co.exitSharePrice > 0) {
                shares = Companies.shares(co.id);
                val += shares * co.exitSharePrice;
            }
        }
    }
    // console.debug("calcPredictedValue: "+ Math.round(val));
    return(Math.round(val));

}

function countCompanies(filter) {
    var count = 0;
    for (c in Companies.map) {
        co = Companies.map[c];
        if (filter == "all" || co.status == filter)
            count++;
    }
    return(count);
}

function countFunds(filter) {
    var count = 0;
    for (f in Funds.map) {
        fund = Funds.map[f];
        if (filter == "all")
            count++;
        else if (fund.status == filter)
            count++;
        else if (filter == "carry" && (fund.id in FundInvestments.carryMap))
            count++;
        else if (filter == "commit" && (fund.id in FundInvestments.commitMap))
            count++;
    }
    return(count);
}

function calcTotalFundsReturn(filter) {
    var ret = 0;
    var amount = 0;
    for (i = 0; i<FundPayouts.list.length; i++) {
        po = FundPayouts.list[i];
        fund = Funds.fund(po.companyID);
        // console.debug("calcTotalReturn: co:" + co)   // debug
        if (fund != null) {               // shouldn't happen unless there's a bug
            if (po.type == "cash")
                amount = Number(po.cash);
            else
                amount = Math.round(Number(po.shares)*Number(po.sharePrice));
            if (filter == "all" || co.status == filter)
                ret += amount;
        }

    }
    return(ret);
}
function calcTotalFundsCommitted(filter) {
    var ret = 0;
    for (var i=0; i<Funds.list.length; i++) {
        fu = Funds.list[i];
        // console.debug("calctotalfundscommitted (fund,filter) " + angelParens(fu.name, filter));
        if (fu != null) {       // only happens if we have a bug!
            ret += Number(FundInvestments.commit(fu.id));
        }
    }

    return(ret);
}

function calcTotalFundsInvested(filter) {
    var ret = 0;
    for (var i=0; i<FundInvestments.list.length; i++) {
        fu = FundInvestments.list[i];
        // console.debug("calctotalfundscommitted (fund,filter) " + angelParens(fu.name, filter));
        if (fu != null) {       // only happens if we have a bug!
            if (fu.type == "invest") {
                ret += Number(fu.investAmount);
            }
        }
    }
    return(ret);
}
/*
 * A new improved version of getangelData. Call LoadEverything which returns a json with the format:
 *    <table name> : [{entry0 Col1 : entry0 Col1 Value, entry0 Col2 : entry0 Col2 Value, ...}...]
 *    ....
 */
function getAngelData() {
    //console.debug("getAngelData2");

    $.get("investments?loadEverything", function(data,status) {
        // console.debug("getAngelData: " + JSON.stringify(data));
        var allData = JSON.parse(data);
        // console.debug("getAngelData: " + JSON.stringify(allData));


        var sort = "date";
        var property = "inv_date";
        var direction = "DESC";

        Investments.list = allData.investments;
        Investments.list.sort(function(a,b) {
            return investmentSorter(a, b, sort, direction, property);

        });

        TrackerList = allData.tracker;
        TrackedCompanies = allData.trackercompanies;
        TrackedPeople = allData.trackerpeople;
        for (var i=0; i < TrackerList.length; i++) {
            var tr = TrackerList[i];
            TrackerMap[(tr.id).toString()] = tr;
        }
        //console.debug("getAngelData: trackerlist and people: " + angelParens(JSON.stringify(TrackerList), JSON.stringify(TrackedPeople)));

        Companies.parseCompanies(allData.companies);
        // console.debug("getAngelData: number of companies: " + Companies.count + "," + cos.length);
        //console.debug("JSON companies: " + companyData);


        var inv;
        for (var i=0; i < Investments.list.length; i++) {
            inv = Investments.list[i];
            angelReturnsByYear.add(inv, false);   // track angel investments for each year

        }


        property = "payDate";
        Payouts.list = allData.payouts;
        //console.debug("number of payouts: " + Payouts.list.length);
        //console.debug("JSON payouts: " + payoutData);
        Payouts.list.sort(function(a,b) {
            return investmentSorter(a, b, sort, direction, property);

        });
        for (i=0; i < Payouts.list.length; i++) {
            /* value is a pseudo property - not part of payouttable, so we'll create it for the sort */
            po = Payouts.list[i];
            if (po["type"] == "cash")
                po["value"] = po["cash"];
            else
                po["value"] = Number(po["shares"]) * Number(po["sharePrice"]);

            angelReturnsByYear.add(po, true);   // track angel returns for each year
        }

        var notes = allData.valuationnotes;
        Companies.parseNotes(notes);
        /*
         * for each company that has founders do an ajax call (ick) to grab those founders. this will be
         * fast now since few companies will have founders, but eventually we need to do this more
         * efficiently.
         */

        for (i=0; i < Companies.list.length; i++) {
            co = Companies.list[i];


            if (co.founder_ids != "" && co.founder_ids != null && co.founder_ids != 0) {
                //console.debug("getAngelData: i: " + i + " -- founder ids: " + JSON.stringify(co));
                $.get("/companyPage?id="+co.id+"&ajaxFounders", function(data,status) {
                    // console.debug(JSON.stringify(data));
                    var founders = JSON.parse(data);
                    if (founders != null && founders.length > 0) {
                        var coID = founders[0].company_id;


                        //console.debug("getAngelData (coName, coID): company_id || founders " + angelParens(coName, coID)  + ":" + founders[0].company_id + " || " + JSON.stringify(founders));
                        Companies.founders[coID] = founders;
                    }

                });

            }

        }

        var sort = "date";
        var property = "invDate";
        var direction = "DESC";

        // console.debug("getangeldata: fundinvestmentdata: " + JSON.stringify(fundInvestmentData));
        FundInvestments.list = allData.fundinvestments;
        Funds.list.sort(function(a,b) {
            return fundSorter(a, b, sort, direction, property);

        });

        var f;
        for (var i=0; i < FundInvestments.list.length; i++) {
            f = FundInvestments.list[i];
            if (f.type == "commit") {
                if (Number(f.investAmount) > 0)
                    FundInvestments.commitMap[f.fundID] = f;
                if (Number(f.hasCarry))
                    FundInvestments.carryMap[f.fundID] = f;
            }

        }
        // console.debug("loading data: FI.commitMap = " + JSON.stringify(FundInvestments.commitMap));
        // console.debug("loading data: FI.carryMap = " + JSON.stringify(FundInvestments.carryMap));

        Funds.parseCompanies(allData.funds);
        // console.debug("number of funds: " + Funds.count);
        //console.debug("JSON funds: " + fundData);
        /*
         * for every fund model, replace the default model with that model
         */
        var fundModels = allData.fundmodels;
        for (i = 0; i < fundModels.length; i++) {
            model = fundModels[i];
            if (!model.fundID in Funds.map) {
                angelJSerror("Model found for non-existant fund with id: " + model.fundID);
                return;
            }
            // console.debug("getangeldata: setting model for fund (fundID, model)" + angelParens(model.fundID, model.percentPerYear));
            Funds.map[model.fundID].model.setModel(model.percentPerYear);
        }

        property = "payDate";
        FundPayouts.list = allData.fundpayouts;
        // console.debug("number of payouts: " + FundPayouts.list.length);
        //console.debug("JSON payouts: " + fundpayoutData);
        FundPayouts.list.sort(function(a,b) {
            return investmentSorter(a, b, sort, direction, property);

        });
        for (i=0; i < FundPayouts.list.length; i++) {
            /*
             * add value and fundID pseudo properties
             */
            po = FundPayouts.list[i];
            if (po["type"] == "cash")
                po["value"] = po["cash"];
            else
                po["value"] = Number(po["shares"]) * Number(po["sharePrice"]);
            po["fundID"] = po["companyID"];
        }

        // console.debug("getangeldata: notedata:" + JSON.stringify(noteData));
        var notes = allData.fundvaluationnotes;
        Funds.parseFundsNotes(notes);

        console.debug("finishing getAngelData: funds");



    });

}
/*
 * Use get's to read all of the angelcalc data into the angelcalc main page
 */
function getAngelDataDEPRECATED() {

    //console.debug("getAngelData");

    $.get("investments?loadInvestments", function(investmentData,status) {
        $.get("investments?loadCompanies", function(companyData,status) {
            $.get("investments?loadPayouts", function(payoutData,status) {
                $.get("investments?loadValuationNotes", function(noteData,status) {

                    var sort = "date";
                    var property = "inv_date";
                    var direction = "DESC";

                    Investments.list = JSON.parse(investmentData);
                    Investments.list.sort(function(a,b) {
                        return investmentSorter(a, b, sort, direction, property);

                    });


                    Companies.jsonToList(companyData);
                    // console.debug("getAngelData: number of companies: " + Companies.count + "," + cos.length);
                    //console.debug("JSON companies: " + companyData);



                    property = "payDate";
                    Payouts.list = JSON.parse(payoutData);
                    //console.debug("number of payouts: " + Payouts.list.length);
                    //console.debug("JSON payouts: " + payoutData);
                    Payouts.list.sort(function(a,b) {
                        return investmentSorter(a, b, sort, direction, property);

                    });
                    for (i=0; i < Payouts.list.length; i++) {
                        /* this is a pseudo property - not part of payouttable, so we'll create it for the sort */
                        po = Payouts.list[i];
                        if (po["type"] == "cash")
                            po["value"] = po["cash"];
                        else
                            po["value"] = Number(po["shares"]) * Number(po["sharePrice"]);
                    }

                    var notes = JSON.parse(noteData);
                    Companies.parseNotes(notes);
                    /*
                     * for each company that has founders do an ajax call (ick) to grab those founders. this will be
                     * fast now since few companies will have founders, but eventually we need to do this more
                     * efficiently.
                     */

                    for (i=0; i < Companies.list.length; i++) {
                        co = Companies.list[i];


                        if (co.founder_ids != "" && co.founder_ids != null && co.founder_ids != 0) {
                            //console.debug("getAngelData: i: " + i + " -- founder ids: " + JSON.stringify(co));
                            $.get("/companyPage?id="+co.id+"&ajaxFounders", function(data,status) {
                                // console.debug(JSON.stringify(data));
                                var founders = JSON.parse(data);
                                if (founders != null && founders.length > 0) {
                                    var coID = founders[0].company_id;


                                    //console.debug("getAngelData (coName, coID): company_id || founders " + angelParens(coName, coID)  + ":" + founders[0].company_id + " || " + JSON.stringify(founders));
                                    Companies.founders[coID] = founders;
                                }

                            });

                        }

                    }

                    //console.debug("finishing getAngelData: angel");
                    // console.debug("JSON Co: " + JSON.stringify(cos[0]));
                    // console.debug("JSON invest: " + JSON.stringify(Investments[0]));
                });

            });

        });

    });

    $.get("investments?loadFundInvestments", function(fundInvestmentData,status) {
        $.get("investments?loadFunds", function(fundData,status) {
            $.get("investments?loadFundPayouts", function(fundPayoutData,status) {
                $.get("investments?loadFundValuationNotes", function(noteData,status) {
                    $.get("investments?loadFundModels", function(modelData,status) {
                        var sort = "date";
                        var property = "invDate";
                        var direction = "DESC";

                        // console.debug("getangeldata: fundinvestmentdata: " + JSON.stringify(fundInvestmentData));
                        FundInvestments.list = JSON.parse(fundInvestmentData);
                        Funds.list.sort(function(a,b) {
                            return fundSorter(a, b, sort, direction, property);

                        });

                        var f;
                        for (var i=0; i < FundInvestments.list.length; i++) {
                            f = FundInvestments.list[i];
                            if (f.type == "commit") {
                                if (Number(f.investAmount) > 0)
                                    FundInvestments.commitMap[f.fundID] = f;
                                if (Number(f.hasCarry))
                                    FundInvestments.carryMap[f.fundID] = f;
                            }

                        }
                        //console.debug("loading data: FI.commitMap = " + JSON.stringify(FundInvestments.commitMap));

                        Funds.jsonToList(fundData);
                        // console.debug("number of funds: " + Funds.count);
                        //console.debug("JSON funds: " + fundData);
                        /*
                         * for every fund model, replace the default model with that model
                         */
                        var fundModels = JSON.parse(modelData);
                        for (i = 0; i < fundModels.length; i++) {
                            model = fundModels[i];
                            if (!model.fundID in Funds.map) {
                                angelJSerror("Model found for non-existant fund with id: " + model.fundID);
                                return;
                            }
                            // console.debug("getangeldata: setting model for fund (fundID, model)" + angelParens(model.fundID, model.percentPerYear));
                            Funds.map[model.fundID].model.setModel(model.percentPerYear);
                        }

                        property = "payDate";
                        FundPayouts.list = JSON.parse(fundPayoutData);
                        // console.debug("number of payouts: " + FundPayouts.list.length);
                        //console.debug("JSON payouts: " + fundpayoutData);
                        FundPayouts.list.sort(function(a,b) {
                            return investmentSorter(a, b, sort, direction, property);

                        });
                        for (i=0; i < FundPayouts.list.length; i++) {
                            /*
                             * add value and fundID pseudo properties
                             */
                            po = FundPayouts.list[i];
                            if (po["type"] == "cash")
                                po["value"] = po["cash"];
                            else
                                po["value"] = Number(po["shares"]) * Number(po["sharePrice"]);
                            po["fundID"] = po["companyID"];
                        }

                        // console.debug("getangeldata: notedata:" + JSON.stringify(noteData));
                        var notes = JSON.parse(noteData);
                        Funds.parseFundsNotes(notes);

                        console.debug("finishing getAngelData: funds");
                    });
                });
            });
        });

    });

    return true;


}   // getAngelData

function addOrUpdateTracker() {

    var form = document.getElementById("addTrackerForm");

    console.debug("---Add or update Tracker");


    if (addTrackerValidation())
        form.submit();
    else
        angelJSerror("Please enter all required fields. Missing fields are red.")

}
function addTrackerValidation() {
    var allGood = true;

    if ($('#trackerType').val() == "") {
        $('#trtypeHeader').css('color', 'red');
        allGood = false;
    } else
        $('#trtypeHeader').css('color', 'black');

    if ($('#trperson').val() == "" && $('#trcompany').val() == "" ) {
        $('#trcompanyOrPersonHeader').css('color', 'red');
        allGood = false;
    } else
        $('#trcompanyOrPersonHeader').css('color', 'black');

    if ($('#trackerFrequency').val() == "") {
        $('#trfrequencyHeader').css('color', 'red');
        allGood = false;
    } else
        $('#trfrequencyHeader').css('color', 'black');

    if ($('#trackerStart').val() == "") {
        $('#trstartHeader').css('color', 'red');
        allGood = false;
    } else
        $('#trstartHeader').css('color', 'black');


    return allGood;
}

function addOrUpdateInvestment() {
    var form = document.getElementById("addInvestmentForm");

    var inv = document.getElementById("invest_amount");
    var val = document.getElementById("prevaluation");
    var prefShares = document.getElementById("prefShares");
    var commShares = document.getElementById("commonShares");

    console.debug("---Add Investment: inv_amount,value:" + $('#inv_amount').val() + "," +  $('#value').val());

    /*
     * the numbers are formatted thanks to the jquery number formatter so we need to grab the number value to submit
     */
    inv.value =  $('#invest_amount').val();
    val.value =  $('#prevaluation').val();
    prefShares.value =  $('#prefShares').val();
    commShares.value =  $('#commonShares').val();


   if (addInvestmentValidation())
       form.submit();
    else
        angelJSerror("Please enter all required fields. Missing fields are red.")

}
function addInvestmentValidation() {
    var allGood = true;

    if ($('#invest_amount').val() == 0) {
        $('#amountHeader').css('color', 'red');
        allGood = false;
    } else
        $('#amountHeader').css('color', 'black');

    if ($('#value').val() == 0) {
        $('#valHeader').css('color', 'red');
        allGood = false;
    } else
        $('#valHeader').css('color', 'black');

    if ($('#datepickerInv').val() == "") {
        $('#dateHeader').css('color', 'red');
        allGood = false;
    } else
        $('#dateHeader').css('color', 'black');

    if ($('#e5').val() == "") {
        $('#companyHeader').css('color', 'red');
        allGood = false;
    } else
        $('#companyHeader').css('color', 'black');

    if ($('#investType').val() == null) {
        $('#typeHeader').css('color', 'red');
        allGood = false;
    } else
        $('#typeHeader').css('color', 'black');

    if ($('#round').val() == null) {
        $('#roundHeader').css('color', 'red');
        allGood = false;
    } else
        $('#roundHeader').css('color', 'black');

    return allGood;
}

function newInvestRoundToType() {
    console.debug("In newInvestmentRoundToType!!!");
    round = $('#round').val();
    round = round.toLowerCase();

    var roundText = ($('#round').select2('data')).text;
    console.debug('newInvestmentRound testing 4: ' + roundText);

    if (roundText.toLowerCase() == "seed") {
        $('#investType').val("safe");
        $('#valHeader').html('Valuation Cap');
    } else {
        $('#investType').val("equity");
        $('#valHeader').html('Pre-money Valuation');

    }

}
function trackerSorter(a,b, sort, direction, property) {

    if (direction == "DESC") {
        // swap
        var tmp = a;
        a = b;
        b = tmp;
    }

    if (sort == "companyorperson") {
        var namea, nameb;
        if (a["type"] == "company") {
            namea = trackedCompanyIDtoname(a["companyID"]);
        } else
            namea = trackedPersonIDtoname(a["personID"]);

        if (b["type"] == "company") {
            nameb = trackedCompanyIDtoname(b["companyID"]);
        } else
            nameb = trackedPersonIDtoname(b["personID"]);

        return(namea.localeCompare(nameb));
    } else if (sort == "nextdate") {
        d1 = new Date(trackerNextLive(a));
        d2 = new Date(trackerNextLive(b));
        if (d1 > d2)
            return 1;
        if (d2 > d1)
            return -1;

        return 0;
    } else
        return angelSorter(a,b, sort, "ASC", property); // "ASC" because we already swapped for DESC above
}
/******************
 *  Just playing around to see if I can consolidate all the sorting into one function (or maybe one for angel and
 *  one for funds). THIS IS NOT CURRENTLY USED
 *
 * angelSorter returns:
 *                        0   if a = b
 *                        1   if a > b  and direction ASC or a < b and direction DESC
 *                        -1  if a < b  and direction ASC or a > b and direction DESC
 *
 *  possible sorts (how we compare): number, string, date, company, fund, or complex
 *
 *  if not complex, then property is the actual property of object a and b, otherwise it is the
 *  type of complex property. Types of complex properties include:
 *                       roi,invest (total investment),return (total return), shares,
 */

function angelSorter(a,b, sort, direction, property) {
    // console.debug("angelSorter: direction: " + direction + " property: " + property)
    if (direction == "DESC") {
        // swap
        var tmp = a;
        a = b;
        b = tmp;
    }
    if (sort == "number") {
        if (Number(a[property]) > Number(b[property]))
            return 1;
        if (Number(a[property]) < Number(b[property]))
            return -1;

        return 0;
    } else if (sort == "company") {
        // console.debug("investmentSorter: property: " + property);
        return(Companies.map[a[property].toString()].name.localeCompare(Companies.map[b[property].toString()].name));

    } else if (sort == "fund") {
        return(fundsCompare(a[property], b[property]));

    } else if (sort == "string") {
        if (a[property] == null)
            if (b[property] == null)
                return(0);
            else
                return(-1);
        else if (b[property] == null)
            return(1);

        return(a[property].localeCompare(b[property]));
    } else if (sort == "date") {
        d1 = new Date(a[property]);
        d2 = new Date(b[property]);
        if (d1 > d2)
            return 1;
        if (d2 > d1)
            return -1;

        return 0;
    } else if (sort == "complex") {





    }


}


/*
 * 0 = the same
 * 1 a > b
 * -1 b > a
 */
function investmentSorter(a,b, sort, direction, property) {
    // console.debug("investmentSorter: direction: " + direction + " property: " + property)
    if (direction == "DESC") {
        // swap
        var tmp = a;
        a = b;
        b = tmp;
    }
    if (sort == "number") {
        if (Number(a[property]) > Number(b[property]))
            return 1;
        if (Number(a[property]) < Number(b[property]))
            return -1;

        return 0;
    } else if (sort == "company") {
        // console.debug("investmentSorter: property: " + property);
        return(Companies.map[a[property].toString()].name.localeCompare(Companies.map[b[property].toString()].name));

    } else if (sort == "fund") {
        return(fundsCompare(a[property], b[property]));

    } else if (sort == "string") {
        if (a[property] == null)
            if (b[property] == null)
                return(0);
            else
                return(-1);
        else if (b[property] == null)
            return(1);
            
        return(a[property].localeCompare(b[property]));
    } else if (sort == "date") {
        d1 = new Date(a[property]);
        d2 = new Date(b[property]);
        if (d1 > d2)
            return 1;
        if (d2 > d1)
            return -1;

        return 0;
    }



}

function fundsCompare(id1, id2) {
    var aFund, bFund;
    var aval, bval;

    // console.debug("fundsCompare: id1,id2: ", angelParens(id1,id2));

    aFund = Funds.map[id1];
    bFund = Funds.map[id2];

    if (aFund.nonAlphaKey == "yc" && bFund.nonAlphaKey == "yc") {
        aval = Number(aFund.nonAlphaOrder);
        bval = Number(bFund.nonAlphaOrder)

        // console.debug("fundsCompare: (aval,bval):" + angelParens(aval,bval));
        if (aval > bval)
            return 1;
        else if (aval < bval)
            return -1;
        else
            return 0;
    } else
        return(aFund.name.localeCompare(bFund.name));

}

/*
 * 0 = the same
 * 1 a > b
 * -1 b > a
 */
function valueSorter(a,b, sort, direction, property) {
    var aval, bval;
    // console.debug("valueSorter: direction: " + direction + " property: " + property)
    if (property == "invest" || property == "ownership" || property == "shares" || property == "yourvalue" || property == "yourexitvalue" || property == "return" || property == "roi") {
        /*
         * this is a custom calc.
         */
        if (direction == "DESC") {
            // swap
            var tmp = a;
            a = b;
            b = tmp;
        }

        if (property == "invest") {
            aval = Number(Companies.invested(a["id"]));
            bval = Number(Companies.invested(b["id"]));
            if (aval > bval)
                return 1;
            else if (aval < bval)
                return -1;
            else
                return 0;
        } else if (property == "return") {
            aval = Number(Companies.returned(a["id"], Payouts));
            bval = Number(Companies.returned(b["id"], Payouts));
            if (aval > bval)
                return 1;
            else if (aval < bval)
                return -1;
            else
                return 0;
        } else if (property == "roi") {
            aval = Number(Companies.returned(a["id"], Payouts))/Number(Companies.invested(a["id"]));
            bval = Number(Companies.returned(b["id"], Payouts))/Number(Companies.invested(b["id"]));
            if (aval > bval)
                return 1;
            else if (aval < bval)
                return -1;
            else
                return 0;
        } else if (property == "shares") {
            aval = Number(Companies.shares(a["id"]));
            bval = Number(Companies.shares(b["id"]));
            // console.debug("valueSorter - shares (a,b): " + angelParens(aval,bval));
            if (aval > bval)
                return 1;
            else if (aval < bval)
                return -1;
            else
                return 0;
        } else if (property == "ownership") {
            fdsharesa = Number(a["FDshares"]);
            fdsharesb = Number(b["FDshares"]);
            if (fdsharesa == 0)
                if (fdsharesb == 0)
                    return 0;
                else
                    return -1;
            else if (fdsharesb == 0)
                return(1);

            aval = (Number(Companies.shares(a["id"])) / fdsharesa);
            bval = (Number(Companies.shares(b["id"])) / fdsharesb);
            console.debug("valueSorter - ownership (namea,nameb)(a,b): " + angelParens(Companies.map[a["id"].toString()].name,Companies.map[b["id"].toString()].name) +
                                    angelParens(aval,bval));
            if (aval > bval)
                return 1;
            else if (aval < bval)
                return -1;
            else
                return 0;
        } else if (property == "yourvalue") {
            // fdsharesa = Number(a["FDshares"]);
            // fdsharesb = Number(b["FDshares"]);
            var sharePriceA = Number(a["sharePrice"]);
            var sharePriceB = Number(b["sharePrice"]);
            // console.debug("sorting values on yourvalue: " + angelParens(sharePriceA, sharePriceB));
            if (sharePriceA == 0)
                if (sharePriceB == 0)
                    return 0;
                else
                    return -1;
            else if (sharePriceB == 0)
                return(1);

            // aval = (Number(Companies.shares(a["id"])) / fdsharesa) * Number(a["valuation"]);
            // bval = (Number(Companies.shares(b["id"])) / fdsharesb) * Number(b["valuation"]);
            aval = Number(Companies.shares(a["id"])) * sharePriceA;
            bval = Number(Companies.shares(b["id"])) * sharePriceB;
           /* console.debug("valueSorter - yourvalue (namea,nameb)(a,b): " + angelParens(Companies.map[a["id"].toString()].name,Companies.map[b["id"].toString()].name) +
                                                        angelParens(aval,bval));
                                                        */
            if (aval > bval)
                return 1;
            else if (aval < bval)
                return -1;
            else
                return 0;
        } else if (property == "yourexitvalue") {
            var sharePriceA = Number(a["exitSharePrice"]);
            var sharePriceB = Number(b["exitSharePrice"]);
            var sharesA = Number(Companies.shares(a["id"]));
            var sharesB = Number(Companies.shares(b["id"]));

            //console.debug("sorting values on yourvalue: " + angelParens(Companies.map[a["id"].toString()].name,Companies.map[b["id"].toString()].name) + angelParens(sharePriceA, sharePriceB) + angelParens(sharesA, sharesB));
            if (sharePriceA == -1 || sharesA == 0)
                if (sharePriceB == -1 || sharesB == 0)
                    return(0);
                else
                    return(-1);
            else if (sharePriceB == -1 || sharesB == 0)
                return(1);

   /*         if (sharePriceA == 0)
                if (sharePriceB == 0)
                    return 0;
                else
                    return -1;
            else if (sharePriceB == 0)
                return(1);*/

            aval = sharesA * sharePriceA;
            bval = sharesB * sharePriceB;
            /*console.debug("valueSorter - yourvalue (namea,nameb)(a,b): " + angelParens(Companies.map[a["id"].toString()].name,Companies.map[b["id"].toString()].name) +
             angelParens(aval,bval));*/

            if (aval > bval)
                return 1;
            else if (aval < bval)
                return -1;
            else
                return 0;
        }
    } else {
        console.debug("valueSorter calling investmentSorter: direction: " + direction + " property: " + property);
        return(investmentSorter(a,b,sort,direction, property));
    }

}   // valuesorter

/*
 * 0 = the same
 * 1 a > b
 * -1 b > a
 */
function fundSorter(a,b, sort, direction, property) {
    var aval, bval;
    var aFund, bFund;
    // console.debug("fundSorter: direction: " + direction + " property: " + property)

    if (direction == "DESC") {
        // swap
        var tmp = a;
        a = b;
        b = tmp;
    }

    // don't get confused: aFund and bFund are fund investments not funds!

    if (a["id"] in FundInvestments.commitMap)
        aFund = FundInvestments.commitMap[a["id"]];
    else
        aFund = FundInvestments.carryMap[a["id"]];

    if (b["id"] in FundInvestments.commitMap)
        bFund = FundInvestments.commitMap[b["id"]];
    else
        bFund = FundInvestments.carryMap[b["id"]];



    if (property == "id") {
        // in this case actually compare the fund name, not properties of the fund investments
        aFund = Funds.map[a["id"]];
        bFund = Funds.map[b["id"]];
        // console.debug("fundsorter: (aFund,bFund) " + angelParens(JSON.stringify(aFund), JSON.stringify(bFund)));
        if (aFund.nonAlphaKey == "yc" && bFund.nonAlphaKey == "yc") {
            aval = Number(aFund.nonAlphaOrder);
            bval = Number(bFund.nonAlphaOrder)
            // console.debug("fundsorter: (aval,bval!:" + angelParens(aval,bval));
        } else
            return(aFund.name.localeCompare(bFund.name));
        // return(Funds.map[a[property].toString()].name.localeCompare(Funds.map[b[property].toString()].name));

    } else if (property == "notes") {
        var anote = aFund.notes;
        var bnote = bFund.notes;
        if (anote == null)
            if (bnote == null)
                return(0);
            else
                return(-1);
        else if (bnote == null)
            return(1);

        return(anote.localeCompare(bnote));
    } else {
        if (property == "invDate") {
            //console.debug("fundSorter: (a.id,b.id): " + angelParens(a["id"], b["id"]));
            //console.debug("fundSorter: (a.invdate,b.invdate): " + angelParens(aFund.invDate, bFund.invDate));
            aval = new Date(aFund.invDate);
            bval = new Date(bFund.invDate);
        } else if (property == "commit") {
            aval = Number(aFund.investAmount);
            bval = Number(bFund.investAmount);
        } else if (property == "commitPaid") {
            aval = Number(Funds.paidInCapital(a["id"]));
            bval = Number(Funds.paidInCapital(b["id"]));
        } else if (property == "leftOfCommit") {
            aval = Number(aFund.investAmount)- Number(Funds.paidInCapital(a["id"]));
            bval = Number(bFund.investAmount)- Number(Funds.paidInCapital(b["id"]));
        } else if (property == "returned") {
            aval = Number(Funds.returned(a["id"], FundPayouts));
            bval = Number(Funds.returned(b["id"], FundPayouts));
        } else if (property == "ROI") {
            var apaid = Number(Funds.paidInCapital(a["id"]));
            var bpaid = Number(Funds.paidInCapital(b["id"]));
            // console.debug("fundSorter: (apaid, bpaid)" + angelParens(apaid, bpaid));
            if (apaid != 0)
                aval = (Number(Funds.returned(a["id"], FundPayouts)) - apaid) / apaid;
            else
                aval = 0;

            if (bpaid != 0)
                bval = (Number(Funds.returned(b["id"], FundPayouts)) - bpaid) / bpaid;
            else
                bval = 0;
        }
    }
    // console.debug("fundSorter at end: (aval, bval)" + angelParens(aval, bval));
    if (aval > bval)
        return 1;
    else if (aval < bval)
        return -1;
    else
        return 0;

}   // fundsorter

/*
 * 0 = the same
 * 1 a > b
 * -1 b > a
 */
function fundPaymentSorter(a,b, sort, direction, property) {

    var aFund, bFund;
    // console.debug("fundPaymentSorter: sort: " + sort + " direction: " + direction + " property: " + property)

    if (direction == "DESC") {
        // swap
        var tmp = a;
        a = b;
        b = tmp;
    }

    if (a["fundID"] in FundInvestments.commitMap)
        aFund = FundInvestments.commitMap[a["fundID"]];
    else
        aFund = FundInvestments.carryMap[a["fundID"]];

    if (b["fundID"] in FundInvestments.commitMap)
        bFund = FundInvestments.commitMap[b["fundID"]];
    else
        bFund = FundInvestments.carryMap[b["fundID"]];

    if (sort == "number") {
        if (Number(a[property]) > Number(b[property]))
            return 1;
        if (Number(a[property]) < Number(b[property]))
            return -1;

        return 0;
    } else if (sort == "fund") {
        // console.debug("fundpaymentsorter: " + angelParens(JSON.stringify(a), JSON.stringify(b)));
        return(fundsCompare(a["fundID"],b["fundID"]));

    } else if (sort == "string") {
        if (a[property] == null)
            if (b[property] == null)
                return(0);
            else
                return(-1);
        else if (b[property] == null)
            return(1);

        return(a[property].localeCompare(b[property]));
    } else if (sort == "date") {
        var d1 = new Date(a[property]);
        var d2 = new Date(b[property]);
        // console.debug("fundpaymentsorter date: (a,b) (d1,d2) " + angelParens(a[property],b[property]) + angelParens(d1,d2));
        if (d1 > d2)
            return 1;
        if (d2 > d1)
            return -1;

        return 0;
    } else if (sort == 'special') {
        if (property == 'leftOfCommit') {
            aval = Number(aFund.investAmount)- Number(Funds.paidInCapital(aFund.fundID));
            bval = Number(bFund.investAmount)- Number(Funds.paidInCapital(bFund.fundID));
        }

        if (aval > bval)
            return 1;
        else if (aval < bval)
            return -1;
        else
            return 0;
    }



}   // fundpaymentsorter

/*
 * 0 = the same
 * 1 a > b
 * -1 b > a
 */
function fundPerformanceSorter(a,b, sort, direction, property) {

    var aFund, bFund;
    // console.debug("fundPaymentSorter: direction: " + direction + " property: " + property)

    if (direction == "DESC") {
        // swap
        var tmp = a;
        a = b;
        b = tmp;
    }

    if (a["id"] in FundInvestments.commitMap)
        aFundInv = FundInvestments.commitMap[a["id"]];
    else
        aFundInv = FundInvestments.carryMap[a["id"]];

    if (b["id"] in FundInvestments.commitMap)
        bFundInv = FundInvestments.commitMap[b["id"]];
    else
        bFundInv = FundInvestments.carryMap[b["id"]];

    if (sort == "fund") {
        //console.debug("fundperformancesorter: " + angelParens(JSON.stringify(a), JSON.stringify(b)));
        // in this case actually compare the fund name, not properties of the fund investments
        aFund = Funds.map[a["id"]];
        bFund = Funds.map[b["id"]];
        // console.debug("fundsorter: (aFund,bFund) " + angelParens(JSON.stringify(aFund), JSON.stringify(bFund)));
        if (aFund.nonAlphaKey == "yc" && bFund.nonAlphaKey == "yc") {
            aval = Number(aFund.nonAlphaOrder);
            bval = Number(bFund.nonAlphaOrder)
            // console.debug("fundsorter: (aval,bval!:" + angelParens(aval,bval));
        } else
            return(aFund.name.localeCompare(bFund.name));

    } else if (sort == "string") {
        if (a[property] == null)
            if (b[property] == null)
                return(0);
            else
                return(-1);
        else if (b[property] == null)
            return(1);

        return(a[property].localeCompare(b[property]));
    } else if (sort == "date") {
        d1 = new Date(a[property]);
        d2 = new Date(b[property]);
        if (d1 > d2)
            return 1;
        if (d2 > d1)
            return -1;

        return 0;
    } else if (sort == 'special') {
        var areturned = Number(Funds.returned(aFundInv.fundID, FundPayouts));
        var breturned = Number(Funds.returned(bFundInv.fundID, FundPayouts));
        var apaid = Number(Funds.paidInCapital(aFundInv.fundID));
        var bpaid = Number(Funds.paidInCapital(bFundInv.fundID));

        if (property == 'invested') {
            aval = apaid;
            bval = bpaid;
        } else if (property == "returned") {
            aval = areturned;
            bval = breturned;
        } else if (property == "capitalAccount") {
            aval = Number(a["capitalAccount"]);
            bval = Number(b["capitalAccount"]);

        } else if (property == "predictedReturn") {
            aval = Number(a["predictedReturn"]);
            bval = Number(b["predictedReturn"]);

        } else if (property == "actualIRR") {
            if (areturned == 0  || apaid == 0 || areturned < apaid)
                aval = 0;
            else
                aval = calcFundIRR(a, 0);
            if (breturned == 0  || bpaid == 0 ||  breturned < bpaid)
                bval = 0;
            else
                bval = calcFundIRR(b, 0);
            //console.debug("fundperformancesorter: actualIRR " + angelParens(aval, bval));

        } else if (property == "actualTVPI") {
            if (apaid == 0)
                aval = 0;
            else
                aval = areturned / apaid;
            if (bpaid == 0)
                bval = 0;
            else
                bval = breturned / bpaid;

            //console.debug("fundperformancesorter: actualTVPI " + angelParens(aval, bval));

        } else if (property == "TVPIplusCA") {
            if (apaid == 0)
                aval = 0;
            else
                aval = (areturned+Number(a["capitalAccount"])) / apaid;
            if (bpaid == 0)
                bval = 0;
            else
                bval = (breturned+Number(b["capitalAccount"])) / bpaid;

        }  else if (property == "TVPIplusPredict") {
            if (apaid == 0)
                aval = 0;
            else
                aval = (areturned+Number(a["predictedReturn"])) / apaid;
            if (bpaid == 0)
                bval = 0;
            else
                bval = (breturned+Number(b["predictedReturn"])) / bpaid;
        }

        if (aval > bval)
            return 1;
        else if (aval < bval)
            return -1;
        else
            return 0;
    }



}   // fundperformancesorter

function sortToggle(cell) {
    /*
     * TBD - set all other cells to neutral (both arrows)
     */
    for (i=1; i < 7; i++) {
        var co = document.getElementById("angelInvestsHeaderCol"+i);
        if (co != cell)
            co.className = "header";
    }
    if (cell.className != "headerSortDown") {
        cell.className = "headerSortDown";
        return("DESC");
    } else {
        cell.className = "headerSortUp";
        return("ASC");
    }



}

/*
 *************************************************************************
 * add the input investment into the investment table.
 */
function addRowInvestmentTable(inv, index) {
    var table = document.getElementById("investmentsTable");
    var body = table.tBodies[0];
    var row = body.insertRow(-1);
    var notes = "";


    var color;
    var co;

    var d = new Date(inv.inv_date);



    row.style.backgroundColor = RowBackgrounds[index % RowBackgrounds.length];

    co = Companies.map[inv.company_id];

    if (inv.status == "active")
        color = "black";
    else if (inv.status == "dead")
        color = "red";
    else if (inv.status == "acquired")
        color = "green";
    else if (inv.status == "ipo") {
        color = "purple";
    }

    if (inv.type == "safe" || inv.type == "note") {
        if (inv.cap == 0)
            capText = " ?";
        else {
            cap = (Number(inv.cap)/1000000).toFixed(2);
            capText = cap + "mm cap";
        }


        if (inv.converted == true) {
            round =  "seed (" + inv.type + ") @" + capText;
            round = '<div title="Investment was converted">' + round;
            round += '<span  style="color:red">[C]</span></div>';

        } else
            round =  inv.round + " (<a href='javascript:onclick=convertInvestment(" + inv.id + ")'>" + inv.type + "</a>) @" + capText;
    } else {
        if (inv.type == "")
            round =  inv.round;
        else {
            if (inv.type == "option" || inv.type == "warrant") {
                valText = Number(inv.price_paid).toFixed(4);
            } else {
                if (inv.prevaluation == 0)
                    valText = " ?";
                else {
                    value =  Number(inv.prevaluation)/1000000;
                    valText = value.toFixed(2) + "mm"
                }
            }


            round =  inv.round + "(" + inv.type + ") @" + valText;
        }
    }
    var i = -1;

    // console.debug("addrowinvestmentTable: " + inv.company_id);
    row.insertCell(i+1).innerHTML = "<div id=optionsDiv" + inv.id + " class='optionsClass" + inv.id + " optionsArrow'" +
                                    " onclick='showInvestmentOptions(this," + inv.id + ")' class='optionsArrow' style='float:left'></div> &nbsp;" +
                                    "<a href='/companyPage?id=" + inv.company_id + "'>" + Companies.map[(inv.company_id).toString()].name + "</a>";
    row.insertCell(i+2).innerHTML = inv.inv_date;
    row.insertCell(i+3).innerHTML = round;
    if (inv.type == "option" || inv.type == "warrant")
        row.insertCell(i+4).innerHTML = "N/A";
    else
        row.insertCell(i+4).innerHTML = "$" + Number(inv.invest_amount).toLocaleString();

    row.cells[i+4].style.color = color;
    if (inv.shares == 0)
        row.insertCell(i+5).innerHTML = "N/A";
    else {
        hoverText = makeSharesHoverText(inv);

        /* parentDivID = "sharesParent" + inv.id;
        popupDivID = "sharesPopup" + inv.id;

        parentDiv = '<div id="' + parentDivID +  '" onmouseout="hidePopup(this,\'' + popupDivID + '\')" ' +
            '" onmouseover="showPopup(this,\'' + popupDivID + '\')">';
        popupDiv = '<div id="' + popupDivID + '" class="dropdown-content">' + hoverText + '</div>';*/


        parentDiv = '<div title="' + hoverText + '">';

        if (co.FDshares != 0 && co.FDshares != null)
            row.insertCell(i+5).innerHTML = parentDiv +
                Number(inv.shares).toLocaleString() + " (" + (inv.shares/co.FDshares*100).toFixed(2) + "%)</div>";
        else
            row.insertCell(i+5).innerHTML = parentDiv + Number(inv.shares).toLocaleString() +  '</div>';
        /*
        if (co.FDshares != 0 && co.FDshares != null)
            row.insertCell(i+5).innerHTML = parentDiv +
            Number(inv.shares).toLocaleString() + " (" + (inv.shares/co.FDshares*100).toFixed(2) + "%)" + popupDiv + '</div>' ;
        else
            row.insertCell(i+5).innerHTML = parentDiv + Number(inv.shares).toLocaleString() + popupDiv + '</div>';*/

    }
    notes = removeNewlinesAndSlashes(inv.notes);
    if (co.YCbatch != "" && co.YCbatch != null)
        notes = notes + "<span style='color:blue'> YC:" + co.YCbatch + "</span>";

    row.insertCell(i+6).innerHTML = notes;



}   //addRowInvestmentTable

function makeSharesHoverText(inv) {
    var htext = "";
    if (inv.prefShares == 0 && inv.commonShares == 0 && inv.prefSharesSeries == "" )
        htext = "no further information available";
    else {
        psharesDisp = Number(inv.prefShares).toLocaleString();
        csharesDisp = Number(inv.commonShares).toLocaleString();
        if (inv.prefShares != 0 && inv.commonShares == 0) {
            htext = psharesDisp + " preferred shares only";
        } else if (inv.prefShares != 0 && inv.commonShares != 0) {
            htext = psharesDisp + " preferred shares and " + csharesDisp + " common shares";
        } else if (inv.prefShares == 0 && inv.commonShares != 0) {
            htext =  csharesDisp + " common shares only";
        }
    }

    return(htext);
}

// Close the dropdown and other menus if the user clicks outside of it
document.onclick = function(event) {
    var optionClass = ".optionsClass" + SavedOptionID;
    // console.debug(" got a click, optionClass is: " + optionClass);
    if (!event.target.matches(optionClass)) {
        var dropdowns = document.getElementsByClassName("dropdown-content");
        var i;
        for (i = 0; i < dropdowns.length; i++) {
            var openDropdown = dropdowns[i];
            if (openDropdown.classList.contains('show')) {
                openDropdown.classList.remove('show');
            }
        }
    }
    if($('#modelSummaryCaptable').length != 0) {
        $('#modelSummaryCaptable').css("display", "none");      // for model home page
        $('#modelSummaryActionSlider').css("display", "none");
    }
};

function optionAction(action) {

    if (ShowingFriend) {
        $.notify("You cannot edit a friend's investments!", "error");
        return;
    }

    if (action == "editInv") {
        editInvestment(SavedOptionID);
    } else if (action == "deleteInv")
        deleteInvestment(SavedOptionID);
    else if (action == "exitInv")
        exitInvestment(SavedOptionID);
    else if (action == "editPay") {
        editPayout(SavedOptionID);
    } else if (action == "deletePay")
        deletePayout(SavedOptionID);
    else if (action == "editFund")
        editFund(SavedOptionID);
    else if (action == "deleteFund")
        deleteFund(SavedOptionID);
    else if (action == "editFundPayment")
        editFundPayment(SavedOptionID, true);
    else if (action == "addFundPayment")
        editFundPayment(SavedOptionID, false);
    else if (action == "deleteFundPayment")
        deleteFundPayment(SavedOptionID);
    else if (action == "deleteTr")
        deleteTracker(SavedOptionID);
    else if (action == "editTr")
        editTracker(SavedOptionID);
    else
        console.debug("unknown optionAction: " + action);

};

var SavedOptionID = 0;
var SavedContext = "";      // if "funds" option context is funds

function showPopup(element,divID) {
    //console.debug("showSharesPopup: id=" + investID );
    //var pop = document.getElementById("sharesPopup" + investID);
    var pop = document.getElementById(divID);
    pop.style.display = 'block';
    $("#" + divID).css($(element).offset());
}

function hidePopup(element,divID) {
    //console.debug("hideSharesPopup: id=" + investID );
    var pop = document.getElementById(divID);
    pop.style.display = 'none';
}
function showTrackerOptions(element,investID) {
    // console.debug("here 1");
    var drop = document.getElementById("trackerDropdown");

    //console.debug("here: " + element.className);
    // console.debug("investID,pos.top,pos.left: " + investID + "," + $(element).position().top + "," + $(element).position().left);


    SavedOptionID = investID;
    drop.classList.toggle("show");
    $("#trackerDropdown").css($(element).offset());
}

function showInvestmentOptions(element,investID) {
    // console.debug("here 1");
    var drop = document.getElementById("investmentDropdown");

    //console.debug("here: " + element.className);
    // console.debug("investID,pos.top,pos.left: " + investID + "," + $(element).position().top + "," + $(element).position().left);


    SavedOptionID = investID;
    drop.classList.toggle("show");
    $("#investmentDropdown").css($(element).offset());
}

function showPayoutOptions(element,payID,fund) {

    var drop = document.getElementById("payoutDropdown");

    //console.debug("here: " + element.className);
    console.debug("showpayoutoptions: (fund,payID) (pos.top,pos.left): " + angelParens(fund, payID) + angelParens($(element).position().top,$(element).position().left));


    SavedOptionID = payID;
    SavedContext = fund;
    drop.classList.toggle("show");
    $("#payoutDropdown").css($(element).offset());
}

function showFundOptions(element,investID) {
    // console.debug("here 1");
    var drop = document.getElementById("fundDropdown");

    //console.debug("here: " + element.className);
    // console.debug("investID,pos.top,pos.left: " + investID + "," + $(element).position().top + "," + $(element).position().left);


    SavedOptionID = investID;
    drop.classList.toggle("show");
    $("#fundDropdown").css($(element).offset());
}

function showFundPaymentOptions(element,id) {
    // console.debug("here 1");
    var drop = document.getElementById("fundPaymentDropdown");

    //console.debug("here: " + element.className);
    // console.debug("investID,pos.top,pos.left: " + investID + "," + $(element).position().top + "," + $(element).position().left);


    SavedOptionID = id;
    drop.classList.toggle("show");
    $("#fundPaymentDropdown").css($(element).offset());
}

function clearInvestmentTable() {

    $("#investmentsTable tbody tr").remove();
}

function editTracker(id) {

    console.debug("editTracker id: " + id);
    var tr = TrackerMap[id.toString()];

    if (tr == 0) {
        console.debug("editTracker called with unknown id");
        angelJSerror("There is a problem editing the tracker: this tracker is missing. Please contact support.");
        return;
    }

    var addDiv = document.getElementById('addTrackerDiv');
    document.getElementById("newTrackerButton").innerHTML = "Update This Tracker";

    $("#trackerType").select2("val", tr.type);
    if (tr.type == "company") {
        $('#trpersonDiv').css('display','none');
        $('#trcompanyDiv').css('display','block');
        $('#trcompanyOrPersonHeader').html('Company<span style=\"color:red\">*</span></th>');
        $("#trcompany").select2("val", null);
        $("#trcompany").select2("val", tr.companyID);
    } else {
        $('#trpersonDiv').css('display','block');
        $('#trcompanyDiv').css('display','none');
        $('#trcompanyOrPersonHeader').html('Person<span style=\"color:red\">*</span></th>');
        $("#trperson").select2("val", null);
        $("#trperson").select2("val", tr.personID);
    }

    $("#trackerFrequency").select2("val", tr.frequency);
    $("#trackerStart").select2("val", tr.start);

    notes = removeNewlinesAndSlashes(tr.notes);
    $("#trackernotes").val(notes);

    $('#trackeraction').val("update");
    $('#trackerupdate').val(tr.id);

    showAddForm("tracker");
    hideGraphics();

}   // editTracker

function deleteTracker(id) {
    // var tr = $(t).closest("tr")
    var id, name;
    console.debug("deleteTracker id: " + id);
    var tr = TrackerMap[id.toString()];

    if (tr == 0) {
        console.debug("deleteTracker called with unknown id");
        angelJSerror("There is a problem deleting the tracker: this tracker is missing. Please contact support.");
        return;
    }

    if (tr.type == "person") {
        id = tr.personID;
        name = trackedPersonIDtoname(id);
    } else if (tr.type == "company") {
        id = Number(tr.companyID);
        name = trackedCompanyIDtoname(id);
    } else {
        name = "oops, not a person or company??";
    }

    var trName = document.getElementById('trackerToDelete');
    trName.innerHTML = "<span style='color:blue'>" + name + "</span>";

    $("#dialogDeleteTracker").dialog({
        resizable: false,
        height: 200,
        width: 400,
        modal: true,
        buttons: {
            Proceed: function() {
                proceed(tr);
                $(this).dialog("close");
            },
            Cancel: function() {
                $(this).dialog("close");
            }
        }
    });

    function proceed(tr){
        $.get("tracker?id=" + tr.id + "&ajaxDeleteTracker", function(data,status) {
            for (var i=0; i < TrackerList.length; i++) {
                entry = TrackerList[i];
                if (entry.id == tr.id) {
                    TrackerList.splice(i, 1);
                    break;
                }
            }
            showTracker(InvestorID);
            $.notify("Tracker Deleted", 'success');

        });
    }

}   // deleteTracker


function deleteInvestment(id) {
    // var tr = $(t).closest("tr")
    console.debug("deleteInvestment id: " + id);
    var inv = Investments.findByID(id);

    if (inv == 0) {
        console.debug("deleteInvestment called with unknown id");
        angelJSerror("There is a problem deleting the investment: this investment is missing. Please contact support.");
        return;
    }

    payouts = Payouts.findByCompany(inv.company_id);
    if (payouts != null) {
        angelJSerror("You cannot delete an investment for which there are distributions!");
        return;
    }

    var invName = document.getElementById('investToDelete');
    invName.innerHTML = "<span style='color:blue'>" + Companies.map[(inv.company_id).toString()].name + "</span>";



    $("#dialogDelete").dialog({
        resizable: false,
        height: 200,
        width: 400,
        modal: true,
        buttons: {
            Proceed: function() {
                proceed(inv);
                $(this).dialog("close");
            },
            Cancel: function() {
                $(this).dialog("close");
            }
        }
    });

    function proceed(inv){
        $.get("investments?delete="+inv.id, function(data,status) {
            clearInvestmentTable();     // remove existing investments
            getAngelData();
            showInvestments(inv.investor_id);
            $.notify("Investment Deleted", 'success');

        });
    }

}   // deleteInvestment

function deletePayout(id) {
    // var tr = $(t).closest("tr")
    var fund = SavedContext;
    var po;

    console.debug("deletePayout (id,fund): " + angelParens(id,fund));
    if (fund == "fund")
        po = FundPayouts.findByID(id);
    else
        po = Payouts.findByID(id);

    if (po == 0) {
        console.debug("deletePayout called with unknown id");
        angelJSerror("There is a problem: this payout is missing. Please contact support.");
        return;
    }

    var poName = document.getElementById('payoutToDelete');
    if (fund == "fund")
        poName.innerHTML = "<span style='color:blue'>" + Funds.map[(po.companyID).toString()].name + "</span>";
    else
        poName.innerHTML = "<span style='color:blue'>" + Companies.map[(po.companyID).toString()].name + "</span>";



    $("#dialogDeletePayout").dialog({
        resizable: false,
        height: 200,
        width: 400,
        modal: true,
        buttons: {
            Proceed: function() {
                proceed(po,fund);
                $(this).dialog("close");
            },
            Cancel: function() {
                $(this).dialog("close");
            }
        }
    });

    function proceed(po,fund){
        var url;
        if (fund == "fund")
            url = "investments?deletePayout="+po.id + "&fund=0";
        else
            url = "investments?deletePayout="+po.id;

        console.debug("deletePayout, proceed: url:" + url);
        $.get(url, function(data,status) {
            clearPayoutTable(fund);     // remove existing payouts
            getAngelData();
            $(document).ajaxStop(function() {
                if (fund == "fund")
                    showFundPayouts(po.investorID);
                else
                    showPayouts(po.investorID);
                $('#messageDiv').notify("Distribution Deleted", 'success');
                console.debug("return from deletePayout: " + data);
            });

        });
    }

}   // deletePayout

function deleteFund(id) {
    // var tr = $(t).closest("tr")
    console.debug("deleteFund id: " + id);
    var inv = FundInvestments.findByID(id);

    if (inv == 0) {
        console.debug("deleteFund called with unknown id");
        angelJSerror("There is a problem deleting the fund investment: this investment is missing. Please contact support.");
        return;
    }

    payouts = FundPayouts.findByCompany(inv.fundID);
    if (payouts != null) {
        angelJSerror("You cannot delete a fund investment for which there are distributions!");
        return;
    }

    var fundName = document.getElementById('fundToDelete');
    fundName.innerHTML = "<span style='color:blue'>" + Funds.map[(inv.fundID).toString()].name + "</span>";


    $("#dialogDeleteFund").dialog({
        resizable: false,
        height: 200,
        width: 400,
        modal: true,
        buttons: {
            Proceed: function() {
                proceed(inv);
                $(this).dialog("close");
            },
            Cancel: function() {
                $(this).dialog("close");
            }
        }
    });

    function proceed(inv){
        $.get("investments?fundDelete="+inv.id, function(data,status) {
            clearFundTable();     // remove existing fund table
            getAngelData();
            showFunds(inv.investorID);
            $.notify("Fund Investment Deleted", 'success');

        });
    }

}   // deleteFund

function editFund(id) {


    var inv = FundInvestments.findByID(id);
    var fund = Funds.map[inv.fundID];

    console.debug("editFund!! (id,fund id): " + angelParens(id,fund.id));

    if (inv == 0) {
        console.log("---> editFund called with non-existant id");
        angelJSerror("somehow we lost this investment - cannot edit at the moment");
        return;
    }


    var date = new Date(inv.invDate);
    var options = {timeZone: 'UTC'};
    document.getElementById("newFundInvestmentButton").innerHTML = "Update This Fund Investment";


    $("#fund").select2("val", null);
    $("#fund").select2("val", inv.fundID);

    $("#datepickerFundAdd").datepicker("setDate",date.toLocaleDateString('en-US',options));

    $("#fundKind").val(fund.kind);
    $("#fundCommitAmount").val(inv.investAmount);

    notes = removeNewlinesAndSlashes(inv.notes);
    $("#fundNotes").val(notes);


    $("#hasCarry").prop("checked", Number(inv.hasCarry));

    console.debug("editFund: hasCarry: " + inv.hasCarry);

    $("#wireBankName").val(fund.wireBank);
    $("#wireBankAddress").val(fund.wireBankAddress);
    $("#wireBankRouting").val(fund.wireBankRouting);
    $("#wireBankSWIFT").val(fund.wireBankSwift);
    $("#wireBankAcct").val(fund.wireBankAcct);
    $("#wireFCName").val(fund.wireFurtherName);
    $("#wireFCAcct").val(fund.wireFurtherAcct);


    $('#invWiringDetails').css('display','block');
    $('#enterWiringText').css('display','none');

    $('#fundAction').val("updateFund");
    $('#fundUpdate').val(inv.id);

    showAddForm("fundinvestment");
    hideGraphics();

}   // editFund

function deleteFundPayment(id) {
    // var tr = $(t).closest("tr")
    console.debug("deleteFundPayment id: " + id);
    var inv = FundInvestments.findByID(id);

    if (inv == 0) {
        console.debug("deleteFundPayment called with unknown id");
        angelJSerror("There is a problem deleting the fund investment: this investment is missing. Please contact support.");
        return;
    }

    var fundName = document.getElementById('fundToDelete');
    fundName.innerHTML = " payment to " + "<span style='color:blue'>" + Funds.map[(inv.fundID).toString()].name + "</span>";


    $("#dialogDeleteFund").dialog({
        resizable: false,
        height: 200,
        width: 400,
        modal: true,
        buttons: {
            Proceed: function() {
                proceed(inv);
                $(this).dialog("close");
            },
            Cancel: function() {
                $(this).dialog("close");
            }
        }
    });

    function proceed(inv){
        $.get("investments?fundDelete="+inv.id, function(data,status) {
            clearFundPaymentsTable();     // remove existing fund table
            getAngelData();
            showFundPayments(inv.investorID);
            $.notify("Fund Payment Deleted", 'success');

        });
    }

}   // deleteFund

/*
 * show edit fund payment form, but if editing if alse, it's a new payment not an edit
 */
function editFundPayment(id, editing) {


    var inv = FundInvestments.findByID(id);
    var fund = Funds.map[inv.fundID];

    console.debug("editFundPayment!! (id,fund id): " + angelParens(id,fund.id));

    if (inv == 0) {
        console.log("---> editFundPayment called with non-existant id");
        angelJSerror("somehow we lost this investment - cannot edit at the moment");
        return;
    }
    var date;
    if (editing)
        date = new Date(inv.invDate);
    else
        date = new Date();
    var options = {timeZone: 'UTC'};



    $("#paymentfund").select2("val", null);
    $("#paymentfund").select2("val", inv.fundID);

    $("#datepickerFundPayment").datepicker("setDate",date.toLocaleDateString('en-US',options));

    $("#fundpaymentAmount").val(inv.investAmount);

    notes = removeNewlinesAndSlashes(inv.notes);
    $("#fundpaymentNotes").val(notes);

    if (editing) {
        $('#fundpaymentAction').val("updateFundPayment");
        $('#fundpaymentUpdate').val(inv.id);
        document.getElementById("newFundPaymentButton").innerHTML = "Update This Fund Payment";
    } else {
        $('#fundpaymentAction').val("addFundPayment");
        document.getElementById("newFundPaymentButton").innerHTML = "Add New Payment";
    }



    showAddForm("fundpayment");
    hideGraphics();

}   // editFundPayment


/*
 **************************************************************
 *  Clears either a fund payout or an angel investment payout form. Remember, payout = distribution
 */
function clearAddPayoutForm(fund) {

    console.debug("clearAddPayoutForm: " + fund);

    document.getElementById("newPayoutButton" + fund).innerHTML = "Add New Distribution";


    var date = new Date();
    var options = {timeZone: 'UTC'};

    if (fund == "")
        $("#payoutCompany").select2("val", null);
    else
        $("#payoutfund").select2("val", null);

    $("#datepickerPay"+fund).datepicker("setDate",date.toLocaleDateString('en-US',options));


    $("#" + fund + "payoutType").val("cash");


    $('#' + fund + 'payoutAmountHeader').html('Amount<span style=\"color:red\">*</span>');
    $('#' + fund + 'payoutPriceHeader').css('display','none');
    $('#' + fund + 'payoutSymbolHeader').css('display','none');
    $('#' + fund + 'payoutPriceCell').css('display','none');
    $('#' + fund + 'payoutSharesCell').css('display','none');
    $('#' + fund + 'payoutSymbolCell').css('display','none');
    $('#' + fund + 'payoutCashCell').css('display','table-cell');

    $('#' + fund + 'payoutCash').val(0);

    /* stock! */

    $('#' + fund + 'payoutShares').val(0);
    $('#' + fund + 'payoutPrice').val(0);
    $('#payoutSymbol').val("");

    $('#' + fund + 'payoutNotes').val("");

    $('#' + fund + 'payoutAction').val('addPayout');
    $('#' + fund + 'payoutTaxInfoDiv').html("");

}
/*
 ****************************************************************************
 * clears a fund payment form
 */
function clearAddFundPaymentForm() {

    console.debug("clearAddPayoutForm");

    document.getElementById("newFundPaymentButton").innerHTML = "Add New Payment";


    var date = new Date();
    var options = {timeZone: 'UTC'};


    $("#paymentfund").select2("val", null);

    $("#datepickerFundPayment").datepicker("setDate",date.toLocaleDateString('en-US',options));

    $('#fundpaymentAmountHeader').html('Amount<span style=\"color:red\">*</span>');

    $('#fundpaymentAmount').val(0);


    $('#fundpaymentNotes').val("");

    $('#fundpaymentAction').val('addFundPayment');

}   // clearAddFundPaymentForm

function editPayout(id) {

    console.debug("editPayout id: " + id);

    var fund = SavedContext;
    document.getElementById("newPayoutButton"+fund).innerHTML = "Update This Distribution";

    /*
     *
     */
    if (fund == "fund")
        po = FundPayouts.findByID(id);
    else
        po = Payouts.findByID(id);

    var date = new Date(po.payDate);
    var options = {timeZone: 'UTC'};

    if (fund == "fund") {
        $("#payoutfund").select2("val", null);
        $("#payoutfund").select2("val", po.companyID);
    } else {
        $("#payoutCompany").select2("val", null);
        $("#payoutCompany").select2("val", po.companyID);
    }


    $("#datepickerPay"+fund).datepicker("setDate",date.toLocaleDateString('en-US',options));


    $("#" + fund + "payoutType").val(po.type);
    //$("#inv_amount").value = inv.invest_amount;

    if (po.type == "cash") {
        $("#" + fund + 'payoutAmountHeader').html('Amount<span style=\"color:red\">*</span>');
        $("#" + fund + 'payoutPriceHeader').css('display','none');
        $("#" + fund + 'payoutSymbolHeader').css('display','none');
        $("#" + fund + 'payoutPriceCell').css('display','none');
        $("#" + fund + 'payoutSharesCell').css('display','none');
        $("#" + fund + 'payoutSymbolCell').css('display','none');
        $("#" + fund + 'payoutCashCell').css('display','table-cell');

        $("#" + fund + "payoutCash").val(po.cash);
    } else {
        /* stock! */
        $("#" + fund + 'payoutAmountHeader').html('No. Shares<span style=\"color:red\">*</span>');
        $("#" + fund + 'payoutPriceHeader').css('display','table-cell');
        $("#" + fund + 'payoutSymbolHeader').css('display','table-cell');
        $("#" + fund + 'payoutPriceCell').css('display','table-cell');
        $("#" + fund + 'payoutSharesCell').css('display','table-cell');
        $("#" + fund + 'payoutSymbolCell').css('display','table-cell');
        $("#" + fund + 'payoutCashCell').css('display','none');

        $("#" + fund + "payoutShares").val(po.shares);
        $("#" + fund + "payoutPrice").val(po.sharePrice);
        $("#" + fund + "payoutSymbol").val(po.symbol);

    }

    var notes = removeNewlinesAndSlashes(po.notes);
    $("#" + fund + "payoutNotes").val(notes);

    $("#" + fund + 'payoutAction').val("updatePayout");
    $("#" + fund + 'payoutUpdate').val(po.id);

    payoutAddTaxInfo(po.id);

    console.debug("Edit payout: po.basis: " + po.basis);

    hideGraphics();
    showAddForm(fund+"payout");
}

function clearAddInvestmentForm() {

    var inv = 0;

    console.debug("clearAddInvestmentForm");

    var date = new Date();
    var options = {timeZone: 'UTC'};
    document.getElementById("newInvestmentButton").innerHTML = "Add New Investment";


    $("#e5").select2("val", null);

    $("#datepickerInv").datepicker("setDate",date.toLocaleDateString('en-US',options));
    $("#round").val("");
    $("#investType").val("");
    $("#invest_amount").val(0);
    $("#prevaluation").val(0);
    $('#valHeader').html("Valuation Cap");

    $("#round").select2("val", null);

    $("#notes").val("");

    $('#action').val("add");
    $("#prefShares").val(0);
    $("#commonShares").val(0);

    $("#prefSharesSeries").select2("val", null);
    $("#prefCertNum").val("");
    $("#commCertNum").val("");
    $("#certLoc").val("");
    $("#converted").prop('checked', false);

    $('#invEquityDetails').css('display','none');
    $('#enterEquityText').css('display','block');

}

function clearTrackerForm() {

    var inv = 0;

    console.debug("clearTrackerForm");

    var addDiv = document.getElementById('addTrackerDiv');
    document.getElementById("newTrackerButton").innerHTML = "Add New Tracker";


    $('#trpersonDiv').css('display','none');
    $('#trcompanyDiv').css('display','block');
    $('#trcompanyOrPersonHeader').html('Company<span style=\"color:red\">*</span></th>');
    $("#trcompany").select2("val", null);
    $("#trackerType").select2("val", null);

    $("#trackerFrequency").select2("val", null);
    $("#trackerStart").select2("val", null);

    $("#trackernotes").val("");


}

function editInvestment(id) {

    var inv = 0;



    inv = Investments.findByID(id);

    console.debug("editInvestment!! (id,round): " + angelParens(id,inv.round) + " Round: " + RoundMap[(inv.round).toLowerCase()]);

    if (inv == 0) {
        console.log("---> editInvestment called with non-existant id");
        angelJSerror("somehow we lost this investment - cannot edit at the moment");
        return;
    }


    var addDiv = document.getElementById('addInvestmentDiv');
    var date = new Date(inv.inv_date);
    var options = {timeZone: 'UTC'};
    document.getElementById("newInvestmentButton").innerHTML = "Update This Investment";


    $("#e5").select2("val", null);
    $("#e5").select2("val", inv.company_id);
    $("#round").select2("val", null);

    // although stored in DB with ids, in inv we leave rounds as text so need to map back!
    $("#round").select2("val", RoundMap[(inv.round).toLowerCase()]);
    $("#prefSharesSeries").select2("val", null);
    $("#prefSharesSeries").select2("val", RoundMap[(inv.prefSharesSeries).toLowerCase()]);

    $("#datepickerInv").datepicker("setDate",date.toLocaleDateString('en-US',options));

    $("#investType").val(inv.type);
    $("#invest_amount").val(inv.invest_amount);
    if (inv.type == "option" || inv.type == "warrant") {
        $("#prevaluation").val(inv.price_paid);
        $('#valHeader').html('Strike Price');
        $('#amountHeader').html('Shares');
    } else if (inv.type == "safe" || inv.type == "note") {
        $("#prevaluation").val(inv.cap);
        $('#valHeader').html('Valuation Cap');
        $('#amountHeader').html('Amount');
    } else {
        $("#prevaluation").val(inv.prevaluation);
        $('#valHeader').html('Pre-money Valuation');
        $('#amountHeader').html('Amount');
    }

    notes = removeNewlinesAndSlashes(inv.notes);
    $("#notes").val(notes);

    $('#action').val("update");
    $('#update').val(inv.id);

    if (inv.prefShares != 0)
        $("#prefShares").val(inv.prefShares);
    else
        $("#prefShares").val("");

    if (inv.commonShares != 0)
        $("#commonShares").val(inv.commonShares);
    else
        $("#commonShares").val("");

    if ((inv.prefShares == 0 && inv.commonShares == 0))
        $("#prefShares").val(inv.shares);

    $("#prefCertNum").val(inv.prefCertNum);
    $("#commCertNum").val(inv.commCertNum);
    $("#certLoc").val(inv.certLoc);

    // console.debug("editInvestment: inv.converted!: " + inv.converted);
    if (inv.converted == true)
        $("#converted").prop('checked', true);
    else
        $("#converted").prop('checked', false);

    $('#invEquityDetails').css('display','block');
    $('#enterEquityText').css('display','none');

    showAddForm("investment");
    hideGraphics();

}   // editInvestment

function exitInvestment(id) {
    console.debug("exitInvestment id: " + id);

    inv = Investments.findByID(id);

    if (inv == 0) {
        console.log("---> exitInvestment called with non-existant id");
        angelJSerror("somehow we lost this investment - cannot record exit at the moment");
        return;
    }

    var invName = document.getElementById('investToExit');
    invName.innerHTML = Companies.map[(inv.company_id).toString()].name;
    var date = new Date();
    var options = {timeZone: 'UTC'};
    var companyName = Companies.company(inv.company_id).name;

    // $("#datepickerExit").datepicker("setDate",date.toLocaleDateString('en-US',options));
    $("#datepickerExit").datepicker();
    $('#exitPrice').number(true);

    $("#dialogExit").dialog({
        resizable: false,
        height: 400,
        width: 550,
        modal: true,
        buttons: {
            Proceed: function() {
                proceed(inv);
                $(this).dialog("close");
                $.notify("Exit for '" + companyName + "' confirmed.", "success");
            },
            Cancel: function() {
                $(this).dialog("close");

            }
        }
    });

    function proceed(inv){

        var exit = $('input[name=exitType]:checked').val();
        var date = $('#datepickerExit').val();
        var acq = $('input[name=exitAcquirer]').val();
        var price = $('input[name=exitPrice]').val();

        var companyName = Companies.company(inv.company_id).name;

        // console.debug("exiting (proceed) exit(date) acquirer/price is: " + exit + "(" + date + ") " + acq + "/" + price);

        var url = "investments?exit=" + exit + "&id=" + inv.id + "&date="+ date + "&acquirer=" + acq + "&price=" + price;

        /*
         * this is not correct NEED A NEW DIALOG HERE :(
         */
        if (inv.status != "active")
            alert("Are you sure you want to continue? " + companyName + "already had an exit!");

        $.get(url,function(data,status) {
            clearInvestmentTable();     // remove existing investments
            getAngelData();
            showInvestments(inv.investor_id);

            //console.debug("exitInvestment data: " + JSON.stringify(data));
            // console.debug("exitInvestment: status from get: " + status);

        });
    }

}

function convertInvestment(id) {
    console.debug("convertInvestment id: " + id);

    inv = Investments.findByID(id);

    if (inv == 0) {
        console.log("---> convertInvestment called with non-existant id");
        angelJSerror("somehow we lost this investment - cannot convert at the moment");
        return;
    }

    //console.debug("edit investment: inv.converted: " + inv.converted);
    if (inv.converted == true) {
        angelJSerror("warning: convertible was already converted!");
    }
    var invName = document.getElementById('investToConvert');
    var invType = document.getElementById('investToConvertType');
    invName.innerHTML = Companies.map[(inv.company_id).toString()].name;
    invType.innerHTML = inv.type;
   if (inv.type == "note")
        document.getElementById('convInterestRow').style.display = "table-row";
    else
       document.getElementById('convInterestRow').style.display = "none";

    var date = new Date();
    var options = {timeZone: 'UTC'};

    $("#datepickerConvert").datepicker();
    $('#convPrefShares').number(true);
    $('#convCommonShares').number(true);

    $("#dialogConvert").dialog({
        resizable: false,
        height: 450,
        width: 400,
        modal: true,
        buttons: {
            Proceed: function() {
                proceed(inv);
                $(this).dialog("close");
            },
            Cancel: function() {
                $(this).dialog("close");
            }
        }
    });

    function proceed(inv){

        var date = $('#datepickerConvert').val();
        var prefShares = $('input[name=convPrefShares]').val();
        var commShares = $('input[name=convCommonShares]').val();
        var interest = $('input[name=convInterest]').val();
        var series = $('input[name=convSeries]').val();
        var modelID = $('select[name=convModel]').val();

        console.debug("converting -- proceed -- (date,modelid) (prefShares,commShares): series: " + angelParens(date,modelID) + angelParens(prefShares,commShares) + ": " + series);

        var url = "investments?convert" + "&investID=" + inv.id + "&date="+ date + "&prefShares=" + prefShares + "&commonShares=" + commShares + "&prefSeries="
                        + series + "&convertModel=" + modelID + "&interest=" + interest;

        $.get(url,function(data,status) {
            clearInvestmentTable();     // remove existing investments
            getAngelData();
            showInvestments(inv.investor_id);

            console.debug("convertInvestment data: " + JSON.stringify(data));
            console.debug("convertInvestment: status from get: " + status);

            // check to see if successful by looking at the status first!!
            $('#messageDiv').notify('conversion successful!', 'success');


        });
    }

}

/**********************************************
 * Values
 *
 */
function showValues(investorID) {

    console.debug("showValues: investorID is: " + investorID);

    InvestorID = investorID;    // keep investor ID around as global

    showValuesSorted(investorID, 0);

    comboGraphInvestResults("", "summaryGraph1");
    pieGraphInvestStatus("summaryGraph2");
    //comboGraphInvestResults("", "contentSummary");
    // console.debug("summary: " + JSON.stringify(InvestSummary));


}

function showValuesSorted(investorID, cell) {


    var sort = "string";
    var property = "name";
    var direction = "ASC";

    /*
     * if cell is 0, this is the first call when the page is loaded, so the default values above will apply. otherwise...
     */
    if (cell != 0) {
        console.debug("showValuessSorted investorID = " + investorID + " sort=" + cell.getAttribute("mySort") + " prop=" + cell.getAttribute("myProp") + " cellid=" + cell.id);
        sort = cell.getAttribute("mySort");
        property = cell.getAttribute("myProp");
        direction = sortToggle(cell);

        AngelView.saveSort("values", sort,property, direction);

    } else {
        console.debug("showValuesSorted first time investorID = " + investorID);

        AngelView.restore();
        sort = AngelView.sortCol["values"];
        property = AngelView.sortProp["values"];
        direction = AngelView.sortDir["values"];
    }

    clearValuesTable();     // remove existing sort

    Companies.list.sort(function(a,b) {
        return valueSorter(a, b, sort, direction, property);

    });
    var totValue = calcTotalValue();        // for now  counts active only
    var totPredicted = calcPredictedValue();        // for now  counts active only

    console.debug("in showValuesSorted. First company is: " + Companies.list[0].name);
    var count = 0;
    for (i=0; i<Companies.list.length; i++) {
        if (Companies.list[i].status == "active") {
            addRowValuesTable(Companies.list[i], count, totValue, totPredicted);
            count++;
        }
    }
    var totValue = calcTotalValue();        // for now  counts active only
    var totPredicted = calcPredictedValue();        // for now  counts active only
    $("#angelValueTotal").html("$" + totValue.toLocaleString());
    $("#angelPredictedValueTotal").html("$" + totPredicted.toLocaleString());

}


function addRowValuesTable(co, index, totValue, totPredicted) {
    var table = document.getElementById("valuesTable");
    var body = table.tBodies[0];
    var row = body.insertRow(-1);
    var shares;
    var yourValue, yourPercent = "";

    var color;

    row.style.backgroundColor = RowBackgrounds[index % RowBackgrounds.length];

    var bgcolor = row.style.backgroundColor;

    row.addEventListener("mouseover", function() {row.style.backgroundColor = "aliceblue";});
    row.addEventListener("mouseout", function() {row.style.backgroundColor = bgcolor;});



    valuesColMap = {"company":0, "amount":1,"shares":2, "sharePrice": 3, "ownership":4,"companyValue":5,"yourValue":6, "exitSharePrice":7};



    if (co.status == "active")
        color = "black";
    else if (co.status == "dead")
        color = "red";
    else if (co.status == "acquired")
        color = "green";
    else if (co.status == "ipo") {
        color = "purple";
    }


    //console.debug("addrowvaluesTable: " + angelParens(co.id,row.style.backgroundColor));
    row.insertCell(valuesColMap.company).innerHTML =
        "<a href='/companyPage?id=" + co.id + "'>" + Companies.map[(co.id).toString()].name + "</a>";
    row.insertCell(valuesColMap.amount).innerHTML = "$" + Number(Companies.invested(co.id)).toLocaleString();
    shares = Companies.shares(co.id);
    if (shares != 0)
        row.insertCell(valuesColMap.shares).innerHTML = Number(shares).toLocaleString();
    else
        row.insertCell(valuesColMap.shares).innerHTML = "";

    row.cells[valuesColMap.amount].style.color = color;
    if (shares == 0) {
        row.insertCell(valuesColMap.sharePrice).innerHTML = "";
        row.insertCell(valuesColMap.ownership).innerHTML = "";
        yourValue = "";
    } else {
        if (co.sharePrice != 0 && co.sharePrice != null)
            row.insertCell(valuesColMap.sharePrice).innerHTML =
               "$" + Number(co.sharePrice).toFixed(2);
        else
            row.insertCell(valuesColMap.sharePrice).innerHTML = "";

        if (co.FDshares != 0 && co.FDshares != null) {
            row.insertCell(valuesColMap.ownership).innerHTML =
                 (shares/co.FDshares*100).toFixed(2) + "%";             // ownership of entire company!

            if (co.sharePrice != 0 && co.sharePrice != null) {
                yourValue = "$" + Math.round(Number(shares*co.sharePrice)).toLocaleString();
                yourPercent = Number(shares*co.sharePrice*100/totValue).toFixed(0);
            } else {
                yourValue = "$" + Math.round(Number(shares/co.FDshares*co.valuation)).toLocaleString();
                yourPercent = Number((shares/co.FDshares)*co.valuation*100/totValue).toFixed(0);
            }
        } else {
            row.insertCell(valuesColMap.ownership).innerHTML = "";
            if (co.sharePrice != 0 && co.sharePrice != null) {
                yourValue = "$" + Math.round(Number(co.sharePrice * shares)).toLocaleString();
                yourPercent = Math.round(Number(shares*co.sharePrice*100/totValue)).toFixed(0); // percent of your portfolio
            } else
                yourValue = "";
        }
    }
    if (Number(co.valuation) == 0)
        row.insertCell(valuesColMap.companyValue).innerHTML = "";
    else
        row.insertCell(valuesColMap.companyValue).innerHTML = "$" + Number(co.valuation).toLocaleString();

    /*
     * your current value
     */
    if (yourPercent != "")
        row.insertCell(valuesColMap.yourValue).innerHTML = yourValue + " (" + yourPercent + "%)";
    else
        row.insertCell(valuesColMap.yourValue).innerHTML = yourValue;

    //console.debug("in addrow: bgcolor1: " + bgcolor);
    row.cells[valuesColMap.yourValue].addEventListener("click", function() {addValuationUpdate(co.id)});
    row.cells[valuesColMap.yourValue].addEventListener("mouseover", function() {row.cells[valuesColMap.yourValue].style.border = "2px solid blue";});
    row.cells[valuesColMap.yourValue].addEventListener("mouseout", function() {row.cells[valuesColMap.yourValue].style.border = "0px";row.cells[valuesColMap.yourValue].style.borderRight = "2px solid gray";});


    /*
     * your predicted value
     */
    yourValue = "";
    yourPercent = "";
    if (Number(co.exitSharePrice) != -1) {
        if (shares != 0)  {
            yourPercent = Number(co.exitSharePrice*shares*100/totPredicted).toFixed(0);
            yourValue = '<div title="' + "Predicted Price: $" + Number(co.exitSharePrice).toFixed(2) + '">';
            yourValue += "$" + Math.round(Number(co.exitSharePrice)*shares).toLocaleString() + " (" + yourPercent + "%)</div>";

        }

    }

    row.insertCell(valuesColMap.exitSharePrice).innerHTML = yourValue;
    row.cells[valuesColMap.exitSharePrice].style.color = "green";

    row.cells[valuesColMap.exitSharePrice].addEventListener("click", function() {addExitPrediction(co.id)});
    row.cells[valuesColMap.exitSharePrice].addEventListener("mouseover", function() {row.cells[valuesColMap.exitSharePrice].style.border = "2px solid blue";});
    row.cells[valuesColMap.exitSharePrice].addEventListener("mouseout", function() {row.cells[valuesColMap.exitSharePrice].style.border = "0px";row.cells[valuesColMap.exitSharePrice].style.borderRight = "2px solid gray";});


}   //addRowValuesTable

function addValuationUpdate(id) {
    console.debug("addValuationUpdate: id: " + id);


    var coName = document.getElementById('valuationUpdateCompanyName');
    coName.innerHTML = "<span style='color:blue'>" + Companies.map[(id).toString()].name + "</span>";


    $("#dialogValuationUpdate").dialog({
        resizable: false,
        height: 300,
        width: 800,
        modal: true,
        buttons: {
            Proceed: function() {
                proceed(id);
                $(this).dialog("close");
            },
            Cancel: function() {
                $(this).dialog("close");
            }
        }
    });

    function proceed(id){
        var invested = $('input[name=newCompanyInvested]').val();
        var valuation = $('input[name=newCompanyValuation]').val();
        var FDshares = $('input[name=newCompanyFDshares]').val();
        var price = $('input[name=newCompanySharePrice]').val();
        var round = $('input[name=newCompanySeries]').val();

        $.get("companyPage?id="+id+ "&ajaxAddValuationUpdate&invested="+invested+"&valuation="+valuation+"&FDshares="+FDshares+"&price="+price+"&round="+round,
            function(data,status) {
                clearValuesTable();     // remove existing sort
                getAngelData();
                showValues(InvestorID);
                $.notify("Value updated", 'success');
        });
    }

}       // addValuationUpdate

function addExitPrediction(id) {
    console.debug("addExitPrediction: id: " + id);


    var coName = document.getElementById('exitPredictionCompanyName');
    coName.innerHTML = "<span style='color:blue'>" + Companies.map[(id).toString()].name + "</span>";


    $("#dialogExitPrediction").dialog({
        resizable: false,
        height: 200,
        width: 400,
        modal: true,
        buttons: {
            Proceed: function() {
                proceed(id);
                $(this).dialog("close");
            },
            Cancel: function() {
                $(this).dialog("close");
            }
        }
    });

    function proceed(id){
        var exitPrice = $('input[name=newExitSharePrice]').val();

        $.get("companyPage?id="+id+ "&ajaxAddPrediction="+exitPrice, function(data,status) {
            getAngelData();
            $(document).ajaxStop(function(){
                clearValuesTable();     // remove existing sort
                showValues(InvestorID);
                 $.notify("Exit prediction updated", 'success');
            });
        });
    }

}       // addExitPrediction

function clearValuesTable() {

    $("#valuesTable tbody tr").remove();
}

/**********************************************
 * Companies
 *
 */
function showCompanies(investorID) {

    console.debug("showCompanies: investorID is: " + investorID);

    InvestorID = investorID;    // keep investor ID around as global

    showCompaniesSorted(investorID, 0);

    // comboGraphInvestResults("", "summaryGraph1");
    // pieGraphInvestStatus("summaryGraph2");
    graphCompanyValueOverTime("valueOverTimeGraph");
    //comboGraphInvestResults("", "contentSummary");
    // console.debug("summary: " + JSON.stringify(InvestSummary));


}

function showCompaniesSorted(investorID, cell) {

    var companiesContainer = document.getElementById('companiesListContainer');

    var sort = "string";
    var property = "name";
    var direction = "ASC";

    /*
     * if cell is 0, this is the first call when the page is loaded, so the default values above will apply. otherwise...
     */
    if (cell != 0) {
        console.debug("showCompaniesSorted investorID = " + investorID + " sort=" + cell.getAttribute("mySort") + " prop=" + cell.getAttribute("myProp") + " cellid=" + cell.id);
        sort = cell.getAttribute("mySort");
        property = cell.getAttribute("myProp");
        direction = sortToggle(cell);
    } else
        console.debug("showCompaniesSorted investorID = " + investorID);




    Companies.list.sort(function(a,b) {
        return investmentSorter(a, b, sort, direction, property);

    });
    var content = "";


    for (var i=0; i<Companies.list.length; i++) {
        var co = Companies.list[i];
        if ((AngelView.filter == "all" || co.status == AngelView.filter) && ! Companies.isProspect(co.id))
            content += addCompanySummary(co, investorID);
    }

    companiesContainer.innerHTML = content;

}   // showCompaniesSorted()

function showCompanyListSorted(investorID, cell) {

    var sort = "date";
    var property = "inv_date";
    var direction = "DESC";
    var inv;

    /*
     * if cell is 0, this is the first call when the page is loaded, so the default values above will apply.
     * unless cookies are set, in which case cookie values take precedence.
     */

    if (cell != 0 && cell != null) {

        // console.debug("showInvestmentsSorted investorID = " + investorID + " sort=" + cell.getAttribute("mySort") + " prop=" + cell.getAttribute("myProp") + " cellid=" + cell.id);
        sort = cell.getAttribute("mySort");
        property = cell.getAttribute("myProp");
        direction = sortToggle(cell);

        AngelView.saveSort("companies",sort,property, direction);

    } else {
        console.debug("showCompanyListSorted before: (sort,property): " + angelParens(sort, property) + " current filter: " + AngelView.filter);
        AngelView.restore();
        sort = AngelView.sortCol["companies"];
        property = AngelView.sortProp["companies"];
        direction = AngelView.sortDir["companies"];
        console.debug("showCompanyListSorted: (sort,property): " + angelParens(sort, property) + " current filter: " + AngelView.filter);
    }


    $("#companiesTable tbody tr").remove();     // remove existing sort

    Companies.list.sort(function(a,b) {

        return valueSorter(a, b, sort, direction, property);

    });

    var count = 0;
    for (var i=0; i<Companies.list.length; i++) {
        var co = Companies.list[i];
        if ((AngelView.filter == "all" || co.status == AngelView.filter) && ! Companies.isProspect(co.id)) {
            addRowCompanyListTable(co, count);
            count++;
        }
    }

    console.debug("showCompanyListSorted all rows added ");


}

function addCompanySummary(co, investorID) {

    var company;
    var roi;
    var color = "blue";
    var investSummary = "";
    var founders = "";

    // console.debug("addCompanySummary: " + co.name);

   if (co.id in Companies.founders) {
        var needComma = false;
        for (var i=0; i < Companies.founders[co.id].length; i++) {
            var f = Companies.founders[co.id][i];
            if (needComma)
                founders += "," + f.firstname + " " + f.lastname;
            else {
                founders += f.firstname + " " + f.lastname;
                needComma = true;
            }
        }
    }

    // console.debug("addCompanySummary - founders: " + founders);

    company = "<a href='/companyPage?id=" + co.id + "'>";
    roi = Companies.returned(co.id, Payouts)-Companies.invested(co.id);
    if (Companies.invested(co.id) == 0)
        roiPercent = "N/A";
    else
        roiPercent = (Math.round((roi*100)/Companies.invested(co.id))).toLocaleString() + '%';
    if (roi < 0) {
        color = "red";
        roi *= -1;
    }

    if (roi > 1000000)
        roi = Math.round(roi/1000000,2) + "M";

    if (co.shortdesc == null)
        co.shortdesc = "";

    if (co.status != "active") {
        investSummary += '<div class="row"><hr></div>'
                        + '<div class="row">($' + Companies.invested(co.id).toLocaleString() + ")</div>"
                        + '<div class="row">$' + Companies.returned(co.id, Payouts).toLocaleString() + "</div>"
                        + '<div class="row"><hr></div>'
                        + '<div class="row" style="color:' + color + '">$' + (roi).toLocaleString() + " (" + roiPercent + ")</div>";
    } else {
        investSummary += '<div class="row"><hr></div>'
            + '<div class="row">($' + Companies.invested(co.id).toLocaleString() + ")</div>";
    }

    /*
     * we need a wrapper div because div tables ignore max-height and height is only a minimum. with the
     * wrapper we can guarantee that the view stays in a nice grid.
     */
    company += "<div class='companySummaryWrapper'><div class='companySummary'>";
    company += '<div class="row">' + co.name + "</div>";
    if (co.logo != null && co.logo != "") {
        company += '<div class="row"><img height=30 width=100 src="' + co.logo + '"></div>';
    } else {
        company += '<div class="row"><img height=30 width=100 src="' + "img/logos/transblank.png" + '"></div>';
    }

    company +=      '<div class="row" style="color:black; font-size:10px;text-align: right">' + co.status + "</div>"
                    + investSummary
                    + '<div class="row">&nbsp</div>'
                    + '<div class="row" style="color:black; font-size:10px;">' + co.shortdesc + '</div>'
                    + '<div class="row"><hr></div>'
                    + '<div class="row" style="font-size:10px;">' + founders + '</div>'


                + "</div></div>";
    company += "</a>";      // companyPage link

    return(company);

}   //addCompanySummary

function addRowCompanyListTable(co, index) {
    var table = document.getElementById("companiesTable");
    var body = table.tBodies[0];
    var row = body.insertRow(-1);
    var shares;
    var yourValue, yourPercent = "";

    var color;

    row.style.backgroundColor = RowBackgrounds[index % RowBackgrounds.length];

    var bgcolor = row.style.backgroundColor;

    if (co.status == "active")
        color = "black";
    else if (co.status == "dead")
        color = "red";
    else if (co.status == "acquired")
        color = "green";
    else if (co.status == "ipo") {
        color = "purple";
    }

    var invested, returned, roi, shares;
    invested = Math.round(Number(Companies.invested(co.id)));
    returned = Math.round(Number(Companies.returned(co.id, Payouts)));
    shares = Number(Companies.shares(co.id));

    row.insertCell(0).innerHTML = "<a href='/companyPage?id=" + co.id + "'>" + Companies.map[(co.id).toString()].name + "</a>";
    row.insertCell(1).innerHTML = invested.toLocaleString();
    row.insertCell(2).innerHTML = returned.toLocaleString();
    if (invested == 0)
        roi = 0;
    else
        roi = Math.round((returned/invested)*100);

    row.insertCell(3).innerHTML = roi.toLocaleString() + "%";
    row.insertCell(4).innerHTML = shares.toLocaleString();
    if (co.FDshares != 0 && co.FDshares != null)
        row.insertCell(5).innerHTML = (shares/co.FDshares*100).toFixed(2) + "%";
    else
        row.insertCell(5).innerHTML = "";


    // row.cells[i+4].style.color = color;


}

function companyListChooser(element, choice) {

    console.debug("companyListChooser:" + choice);
    /*
     * first reset links
     */
    $("#companyBlockList").css("color","#337ab7");
    $("#companyNameList").css("color","#337ab7");
    $("#companyNameRecency").css("color","#337ab7");
    $("#companyValueOverTime").css("color","#337ab7");


    /*
     * make selected link black (unless we are not called from html) and display correct format
     */
    if (element != null)
        $(element).css("color","black");

    CurrentView.view.subDisplay = choice;
    if (choice == "block") {
        $("#companiesNameListContainer").css("display","none");
        $("#companiesRecentListContainer").css("display","none");
        $("#companiesValueOverTimeContainer").css("display","none");
        $('#companyBlockList').css("color","black");
        $("#companiesListContainer").css("display","block");
        /*
         * nothing more to do, the block of companies is static
         */
    } else if (choice == "list") {
        $("#companiesListContainer").css("display","none");
        $("#companiesRecentListContainer").css("display","none");
        $("#companiesValueOverTimeContainer").css("display","none");
        $('#companyNameList').css("color","black");
        $("#companiesNameListContainer").css("display","block");
        showCompanyListSorted(0, null);
    } else if (choice == "recent") {
        $("#companiesListContainer").css("display","none");
        $("#companiesNameListContainer").css("display","none");
        $("#companiesValueOverTimeContainer").css("display","none");
        $('#companyRecentList').css("color","black");
        $("#companiesRecentListContainer").css("display","block");
    } else if (choice == "value") {
        $("#companiesListContainer").css("display","none");
        $("#companiesNameListContainer").css("display","none");
        $("#companiesRecentListContainer").css("display","none");
        $('#companyValueOverTime').css("color","black");

        $("#companiesValueOverTimeContainer").css("display","inline");
    }
}

/*
 * Printing
 */
function angelPrint(tableName) {

    var win=window.open("");

    if (tableName == "convertSummary") {
        container = document.getElementById("outputDiv");
        win.document.write(container.innerHTML);
        win.print();
        return;
    }

    container = document.getElementById(tableName+"Container");


    win.document.write(container.innerHTML);


    tableDiv = win.document.getElementById(tableName+"TableDiv");
    tableHeader = win.document.getElementById(tableName+"TableHeaderDiv");
    table = win.document.getElementById(tableName+"Table");
    tableDiv.style.height = '';
    tableDiv.style.margin = '0px 0px 0px 20px';
    table.style.textAlign = "right";
    tableHeader.style.margin = '0px 0px 0px 20px';
    if (tableName == "fundPredictions" || tableName == "fundActuals" || tableName == "values") {
        totals = win.document.getElementById(tableName + "TotalsDiv");
        totals.style.margin = '0px 0px 0px 20px';
        totals.style.textAlign = "right";
        totalsTable = win.document.getElementById(tableName + "TotalsTable");
        totalsTable.style.textAlign = "right";
    }

    win.print();
}
/**********************************************
 * Dealflow
 *
 */
function showDealflow(investorID) {

    console.debug("showDealflow: investorID is: " + investorID);

    InvestorID = investorID;    // keep investor ID around as global


    showDealflowSorted(investorID, 0);
    hideGraphics();

    // comboGraphInvestResults("", "summaryGraph1");
    // pieGraphInvestStatus("summaryGraph2");
    //comboGraphInvestResults("", "contentSummary");
    // console.debug("summary: " + JSON.stringify(InvestSummary));


}

function showDealflowSorted(investorID, cell) {

    var companiesContainer = document.getElementById('dealflowContainer');

    var sort = "string";
    var property = "name";
    var direction = "ASC";

    /*
     * if cell is 0, this is the first call when the page is loaded, so the default values above will apply. otherwise...
     */
    if (cell != 0) {
        console.debug("showDealflowSorted investorID = " + investorID + " sort=" + cell.getAttribute("mySort") + " prop=" + cell.getAttribute("myProp") + " cellid=" + cell.id);
        sort = cell.getAttribute("mySort");
        property = cell.getAttribute("myProp");
        direction = sortToggle(cell);

    } else {
        console.debug("showDealflowSorted first call investorID = " + investorID);

    }




    Companies.list.sort(function(a,b) {
        return investmentSorter(a, b, sort, direction, property);

    });
    var content = "";

    for (i=0; i<Investments.list.length; i++) {
        inv = Investments.list[i];

        if (inv.status == "prospect") {
            content += addDealflowCompanySummary(Companies.map[inv.company_id], inv);
        }
    }
    /*
     * if there are no prospects - add a dummy entry which is an exhortation to add one!
     */
    if (content == "") {
        content += addDealflowCompanySummary(null, null);
    }

    companiesContainer.innerHTML = content;

}


function addDealflowCompanySummary(co, inv) {

    var company = "";
    var roi;
    var color = "blue";
    var investSummary = "";
    var founders = "";

    if (co == null) {
        /*
         * add a dummy entry
         */
        company = "<a href='/companyPage?newProspect'>";
        company += "<div class='dealflowCompanySummaryWrapper'><div class='dealflowCompanySummary'>";
        company +=      '<div class="row"><hr></div>' +
                        '<div class="row" style="text-align:center">Add a prospect!</div>' +
                        '<div class="row"><hr></div>';
        company += "</div></div>"
        company += "</a>";      // companyPage link
        return (company);
    }
    // console.debug("addCompanySummary: " + co.name);
    company = "<a href='/companyPage?id=" + co.id + "'>";


    if (co.id in Companies.founders) {

        for (var i=0; i < Companies.founders[co.id].length; i++) {
            founders += '<div class="companyFoundersSummaryDiv">';
            var f = Companies.founders[co.id][i];
            if (f.photo != "")
                founders += "<img alt='Thumb' height='50' width='50' src='" + f.photo + "' /><br/>";
            founders += '<span>' + f.firstname + " " + f.lastname + '</span>';
            founders += '</div>';
        }

    }



    if (co.shortdesc == null)
        co.shortdesc = "";

    investSummary += '<div class="row"><hr></div>'
        + '<div class="row">Proposed Investment: $' + Companies.invested(co.id).toLocaleString() + "</div>"
            + '<div class="row"><hr></div>';


    company += "<div class='dealflowCompanySummary'>";
    company += '<div class="row">' + co.name +
               "<a href='javascript:onclick=deleteProspect(" + co.id + ");'>" +
        "                <div style='width:10px; float:right; text-align: right; color:red'>X</div>" +
                "</a></div>";
    if (co.logo != null && co.logo != "") {
        company += '<div class="row"><img height=30 width=100 src="' + co.logo + '"></div>';
    } else {
        company += '<div class="row"><img height=30 width=100 src="' + "img/logos/transblank.png" + '"></div>';
    }

    company += investSummary
        + '<div class="row">&nbsp</div>'
        + '<div class="row" style="color:black; font-size:10px;">' + co.shortdesc + '</div>'
        + '<div class="row">&nbsp</div>'
        + '<div class="row" style="color:blue; font-size:10px;">' + founders + '</div>'
        + '<div class="row" style="clear:both">&nbsp</div>'
        + '<div class="row"><hr></div>'

        + '<div class="row">&nbsp</div>'
        + '<div class="row" style="bottom:20px;"> <a href="javascript:onclick=editInvestment(' + inv.id + ');">'
        + 'Make an Investment </a></div>'
        + "</div>";     // container
    company += "</a>";      // companyPage link

    return(company);

}   //addDealflowCompanySummary
/*
 * delete this prospect (leave the company, but kill the inv)
 */
function deleteProspect(id) {
    console.debug("deleteProspect. company id is: " + id);
    var invArr = Companies.investments(id);     // returns an array of investments in this company. THere ought to be 1
    var inv = invArr[0];

    var prospectName = document.getElementById('prospectToDelete');
    prospectName.innerHTML = "<span style='color:blue'>" + Companies.map[(id).toString()].name + "</span>";

    $("#dialogDeleteProspect").dialog({
        resizable: false,
        height: 200,
        width: 400,
        modal: true,
        buttons: {
            Proceed: function() {
                proceed(inv);
                $(this).dialog("close");
            },
            Cancel: function() {
                $(this).dialog("close");
            }
        }
    });

    function proceed(inv){
        $.get("investments?delete="+inv.id, function(data,status) {
            // clearDealflowTable();     // remove existing investments
            getAngelData();
            showDealflow(inv.investor_id);
            console.debug("proceed with delete prospect: " + JSON.stringify(data));
            $.notify("Proposed Investment Deleted", 'success');

        });
    }

} // deleteProspect


/***************************************************
 * Dealflow
 **********************************************
 */


/**********************************************
 * Payouts
 *
 */
/*****************************************
 * This function gets called one when the payouts document loads - deprecated in favor of initialize Investments
 * @param investorID
 */
/*function initializePayouts(investorID) {
    addColumnSorters(investorID, 7, showPayoutsSorted);
    showPayouts(investorID);
}*/

function showPayouts(investorID) {

    console.debug("showPayouts: investorID is: " + investorID);

    InvestorID = investorID;    // keep investor ID around as global

    showPayoutsSorted(investorID, 0);

    comboGraphInvestResults("", "summaryGraph1");
    pieGraphInvestStatus("summaryGraph2");

    //comboGraphInvestResults("", "contentSummary");
    // console.debug("summary: " + JSON.stringify(InvestSummary));


}

function showPayoutsSorted(investorID, cell) {

    var contentSummaryDiv = document.getElementById('contentSummary');


    var sort = "date";
    var property = "payDate";
    var direction = "DESC";

    /*
     * if cell is 0, this is the first call when the page is loaded, so the default values above will apply. otherwise...
     */
    if (cell != 0) {
        console.debug("showPayoutsSorted investorID = " + investorID + " sort=" + cell.getAttribute("mySort") + " prop=" + cell.getAttribute("myProp") + " cellid=" + cell.id);
        sort = cell.getAttribute("mySort");
        property = cell.getAttribute("myProp");
        direction = sortToggle(cell);

        AngelView.saveSort("payouts", sort,property, direction);


    } else {
        console.debug("showPayoutsSorted first call investorID = " + investorID);

        AngelView.restore();
        sort = AngelView.sortCol["payouts"];
        property = AngelView.sortProp["payouts"];
        direction = AngelView.sortDir["payouts"];

    }


    clearPayoutTable("");     // remove existing sort

    Payouts.list.sort(function(a,b) {
        return investmentSorter(a, b, sort, direction, property);

    });

    var co;
    var count = 0;
    for (i=0; i<Payouts.list.length; i++) {
        co = Companies.company(Payouts.list[i].companyID);
        if (co != null) {       // only happens if we have a bug!
            if (AngelView.filter == "all" || co.status == AngelView.filter) {
                addRowPayoutTable(Payouts.list[i], count);
                count++;
            }
        }
    }

}
function addRowPayoutTable(po, index) {
    var table = document.getElementById("payoutsTable");
    var body = table.tBodies[0];
    var row = body.insertRow(-1);
    var notes;

    po.type = po.type.toLocaleLowerCase();

    row.style.backgroundColor = RowBackgrounds[index % RowBackgrounds.length];

    var i = -1;

    row.insertCell(i+1).innerHTML = "<div id=optionsDiv" + po.id + " class='optionsClass" + po.id + " optionsArrow'" +
        " onclick='showPayoutOptions(this," + po.id + ",\"\")' class='optionsArrow' style='float:left'></div> &nbsp;" +
        "<a href='/companyPage?id=" + po.companyID + "'>" + Companies.map[(po.companyID).toString()].name + "</a>";
    row.insertCell(i+2).innerHTML = po.payDate;
    row.insertCell(i+3).innerHTML = po.type;
    if (po.type == "cash")
        row.insertCell(i+4).innerHTML = "$" + Number(po.cash).toLocaleString();
    else
        row.insertCell(i+4).innerHTML = "";

    if (po.type == "stock") {       // ack make sure this is stored in lowercase later
        row.insertCell(i+5).innerHTML = Number(po.shares).toLocaleString();
        row.insertCell(i+6).innerHTML = po.symbol;
        row.insertCell(i+7).innerHTML = Number(po.sharePrice).toFixed(4);
        row.insertCell(i+8).innerHTML = "$" + Math.round((Number(po.shares)*Number(po.sharePrice))).toLocaleString();
    } else { // cash
        row.insertCell(i+5).innerHTML = "";
        row.insertCell(i+6).innerHTML = "";
        row.insertCell(i+7).innerHTML = "";
        row.insertCell(i+8).innerHTML = "$" + Number(po.cash).toLocaleString();

    }
    notes = removeNewlinesAndSlashes(po.notes);
    row.insertCell(i+9).innerHTML = notes;

}   //addRowPayoutTable

function clearPayoutTable(fund) {
    if (fund == "fund")
        $("#fundPayoutsTable tbody tr").remove();
    else
        $("#payoutsTable tbody tr").remove();
}


function payoutAddTaxInfo(editing) {
    var id;
    var idLeft;

    var companyID = $('#payoutCompany').val();
    var body = '<p></p><span style="text-align: left; font-size: 12px; display:block">You may optionally record tax information for this distribution. ' +
                'Like any capital gain, your basis in your investments needs to be applied to cash generated from a sale. ' +
                'Usually first in / first out (FIFO) is used, but I always recommend ' +
                'discussing with your tax advisor to make sure you do this in a way the IRS likes. ' +
                'No worries if you don\'t want to deal now or have no idea what these numbers should be. ' +
                'You can always enter this information later.' + '</span><p /><p />';


    if (editing) {
        /* editing is the id of the payout we are editing */
        var basisMap = Payouts.basisToMap(editing);
        console.debug("payoutAddTaxInfo - basisMap: " + JSON.stringify(basisMap));
    }


    var invests = Companies.investments(companyID);
    var totalInvested = 0;
    var value;
    numInvests = invests.length;
    // body += '<div id="savedBasisDiv"></div>';
    body += "<table class='angelPayoutsTaxTable'>";
    body += "<tr><th>Investment Date</th><th>Amount Invested</th><th>Amount to Apply</th><th>Not Yet Applied</th></tr>";
    for (var i = 0; i < numInvests; i++) {
        totalInvested += Number(invests[i].invest_amount);
        id = 'payoutID' + invests[i].id;
        idLeft = id + "Left";
        console.debug("payoutTax: id is: " + id);

        if (editing) {
            if (basisMap == null)
                value = "";
            else
                value = basisMap[invests[i].id];
        } else
            value = "";

        console.debug("payoutAddTaxInfo value=" + value);

        body += "<tr><td>" + invests[i].inv_date + "</td><td>$" + Number(invests[i].invest_amount).toLocaleString()
            + "</td><td><input onchange='doPayoutTaxCalc(" + companyID + "," + editing +  ")' type='text' size='9' id='" + id + "' name='"  + id + "' value=" + value + "></td>" +
            "</td><td><span id='" + idLeft + "' style='font-weight:200'></span></td></tr>";

    }
    body += "<tr><td colspan='3'><hr></td></tr>";
    body += "<tr><td>Total</td><td><span style='font-weight:bold'>$" + totalInvested.toLocaleString() +
        "</span></td><td><span id='payoutTaxTotal' style='font-weight:bold'></span>" +
        "<td><span id='payoutTaxTotalLeft' style='font-weight:bold'></span></td></tr>";
    body += "</table></form>";

    $("#payoutTaxInfoDiv").html(body);
    doPayoutTaxCalc(companyID, editing);

}
function doPayoutTaxCalc(companyID, editing) {
    var poTax = document.getElementById('payoutTaxTotal');
    var poTaxLeft = document.getElementById('payoutTaxTotalLeft');
    var invests = Companies.investments(companyID);
    var invested = Companies.invested(companyID);
    var totalInvested = 0;
    var id, idLeft;

    console.debug("doPayoutTaxCalc: " + companyID);

    if (editing) {
        /* editing is the id of the payout we are editing */
        var basisMap = Payouts.basisToMap(editing);
    }

    numInvests = invests.length;
    total = 0;
    totalLeft = 0;
    for (var i = 0; i < numInvests; i++) {
        id = 'payoutID' + invests[i].id;
        idLeft = id + "Left";
        // idSaved = "payoutIDSaved" + invests[i].id;       // if editing, this stores the current basis used by this payout from this investment

        val = Number(document.getElementById(id).value);
        if (editing) {
            if (basisMap != null)
                valPrev = Number(basisMap[invests[i].id]);
            else
                valPrev = 0;
        } else
            valPrev = 0;

        console.debug("doPayoutTaxCalc: valPrev: " + valPrev);

        investAmount = Number(invests[i].invest_amount);
        investAmountLeft = investAmount - Number(basisUsed(invests[i]));
        investAmountLeft += valPrev;

        console.debug("doPayoutTaxCalc: (investAmount,investAmountLeft): " + angelParens(investAmount, investAmountLeft));


        if (investAmount == 0) {
            document.getElementById(idLeft).innerHTML = "N/A";
        } else {
            if (investAmountLeft - val < 0) {
                $.notify("Oops, you can't apply more basis than you've got!");
                document.getElementById(id).value = "";
                val = 0;
            }
        }
        percent = (investAmountLeft - val)/investAmount*100;
        document.getElementById(idLeft).innerHTML = "$" + (investAmountLeft-val).toLocaleString() + " (" + percent.toFixed(2) + "%)";
        total += val;
        totalLeft += investAmountLeft-val;

    }
    percent = (invested - totalLeft)/invested*100;
    poTax.innerHTML = "$" + total.toLocaleString();
    poTaxLeft.innerHTML = "$" + (totalLeft).toLocaleString() + " (" + percent.toFixed(2) + "%)";

}
/*
 * for tax - calculated total basis used in input investment
 */
function basisUsed(invest) {
    if (invest.basis == null || invest.basis == 0 || invest.basis == "")
        return(0);

    var dict = parseQueryStringToDictionary(invest.basis);
    var basis = 0;

    for (var i in dict) {
        // console.debug("really??: " + i);
        basis += Number(dict[i]);
    }

    console.debug("basisUsed: (invest.basis,returned basis): " + angelParens(invest.basis, basis));
    return(basis);

}

/*
* for tax - calculated total basis allocated to input payout
*/
function basisAllocated(payout) {
    if (payout.basis == null || payout.basis == 0 || payout.basis == "")
        return(0);

    var dict = parseQueryStringToDictionary(payout.basis);
    var basis = 0;

    for (var i in dict) {
        // console.debug("really??: " + i);
        basis += Number(dict[i]);
    }

    console.debug("basisAllocated: (invest.basis,returned basis): " + angelParens(payout.basis, basis));
    return(basis);

}

/*
 ************************************************
 * Friends
 */

/*
 * display investments of the friend represented by
 */
var Friends = {};
var ShowingFriend = false;

function Friend() {this.id=0, this.companies = {}, this.investments = {}, this.payouts = {}};

function testingsomeshit() {
    var x = {list:[], a:"name"};
    function z() {
        this.list = [];
        this.a = "name";
    }
    a = new z();
    b = new z();

    a.list[0] = 1;

    y = Object.create(x);
    console.debug("testing some shit: (b.a, b.list[0])" + angelParens(b.a, b.list[0]));
}

function showFriendInvestments(id, name) {
    var f = {};

    // testingsomeshit();

    if (id in Friends) {
        console.debug("--->showFriendInvestments: id is in Friends: " + id);
        f = Friends[id];
    } else {
        console.debug("--->showFriendInvestments: id not in Friends: " + id);
        f = new Friend();
        f.companies = new CompaniesConstructor();
        f.investments = new InvestmentsConstructor();
        f.payouts = new PayoutsConstructor();

        console.debug("ok  investment (invs list length, Investments list): " +angelParens(Object.keys(f.investments.list).length, Investments.list.length));
        f.id = id;

        Friends[id] = f;
    }

    getFriendData(f);           // don't need to do this every time!
    $(document).ajaxStop(function(){
        Companies = f.companies;
        Investments = f.investments;
        Payouts = f.payouts;            // there should be nothing here!

        $("#friendHeader").html(name);
        $("#friendHeaderDiv").css("display", "block");
        showInvestmentsSorted(f.id, 0, true);
        hideGraphics();
        ShowingFriend = true;

    });

}
function getFriendData(friend) {

    console.debug("getFriendData:" + friend.id);

    $.get("investments?loadInvestments&friendID="+friend.id, function(investmentData,status) {
        $.get("investments?loadCompanies&friendID="+friend.id, function(companyData,status) {

            var sort = "date";
            var property = "inv_date";
            var direction = "DESC";

            friend.investments.list = JSON.parse(investmentData);
            friend.investments.list.sort(function(a,b) {
                return investmentSorter(a, b, sort, direction, property);

            });

            friend.companies.jsonToList(companyData);
            console.debug("number of companies: " + friend.companies.count);
            //console.debug("JSON companies: " + companyData);


            /*
             * for each company that has founders do an ajax call (ick) to grab those founders. this will be
             * fast now since few companies will have founders, but eventually we need to do this more
             * efficiently.
             */
            friend.companies.parseNotes(notes);
            for (i=0; i < friend.companies.list.length; i++) {
                co = friend.companies.list[i];


                if (co.founder_ids != "" && co.founder_ids != null && co.founder_ids != 0) {
                    //console.debug("getAngelData: i: " + i + " -- founder ids: " + JSON.stringify(co));
                    $.get("/companyPage?id="+co.id+"&ajaxFounders", function(data,status) {
                        // console.debug(JSON.stringify(data));
                        var founders = JSON.parse(data);
                        if (founders != null && founders.length > 0) {
                            var coID = founders[0].company_id;


                            //console.debug("getAngelData (coName, coID): company_id || founders " + angelParens(coName, coID)  + ":" + founders[0].company_id + " || " + JSON.stringify(founders));
                            friend.companies.founders[coID] = founders;
                        }

                    });

                }

            }


            // console.debug("JSON Co: " + JSON.stringify(cos[0]));
            // console.debug("JSON invest: " + JSON.stringify(Investments[0]));
        });

    });


    return true;


}   // getFriendData

function showHideFriends() {

    var x = document.getElementById('friendList');
    if (x.style.display === 'none') {
        x.style.display = 'block';
    } else {
        x.style.display = 'none';
    }

}

function showHideAdvisors() {

    var x = document.getElementById('advisorList');
    if (x.style.display === 'none') {
        x.style.display = 'block';
    } else {
        x.style.display = 'none';
    }

}

/**************************************************
 * End of Friends
 */


/**********************************************
 * Start Funds
 *
 */
function addOrUpdateFundNote() {
    var form = document.getElementById("addFundNote");

    var cap = document.getElementById("fundCapitalAccount");
    var ret = document.getElementById("fundPredictedReturn");


    cap.value =  $('#fundCapitalAccount').val();      // the numbers are formatted in val() but not in the text value.
    ret.value =  $('#fundPredictedReturn').val();

    console.debug("addOrUpdateFundNote: (cap, ret) -- " + angelParens(cap.value, ret.value));

    if (addFundNoteValidation())
        form.submit();
    else
        angelJSerror("Please enter all required fields. Missing fields are red.")

}   // addOrUpdateFundNote

function fundNoteTypeSelector() {

    console.debug("fundNoteTypeSelector: type is: " + $("#fundNoteType").val());
    if ($("#fundNoteType").val() == "valuation update") {
        $("#fundValuationDiv").css("display", "inline");
        $("#fundExitPredictionDiv").css("display", "none");
        $("#fundPublic").css("display", "none");
        $("#fundPublicText").html("Note: This information will never be public.");
        $("#fundPublic").prop("checked", 0);
    } else if ($("#fundNoteType").val() == "exit prediction") {
        $("#fundValuationDiv").css("display", "none");
        $("#fundExitPredictionDiv").css("display", "inline");
        $("#fundPublic").css("display", "none");
        $("#fundPublicText").html("Note: This information will never be public.");
        $("#fundPublic").prop("checked", 0);
    }  else {
        $("#fundValuationDiv").css("display", "none");
        $("#fundExitPredictionDiv").css("display", "none");
        $("#fundPublic").css("display", "inline");
        $("#fundPublicText").html("Public? If checked, this note will be visible to other Angelcalc users");
        $("#fundCapitalAccount").val(0);
        $("#fundPredictedReturn").val(0);
    }

}

function fundCashflowModelSelector(selected) {
    // console.debug("fundCashflowModelSelector: selected value is: " + selected.value);
    var dict = parseQueryStringToDictionary(selected.value);
    console.debug("fundCashflowModelSelector: " + JSON.stringify(dict));

    for (i=1;i <= 10; i++) {
        $("#year"+i+"Cashflow").val(dict[i]);
    }
}

function fundCashflowAddPercent() {

    var percentTotal = 0;

    for (i=1;i <= 10; i++) {
        percentTotal += Number($("#year"+i+"Cashflow").val());
    }
    console.debug("fundCashflowAddPercent - percentTotal: " + percentTotal);
    $("#yearsTotalCashflow").html(percentTotal.toString() + " %");
    if (percentTotal != 100) {
        $("#yearsTotalCashflow").css("color", "red");
        $(".ui-dialog-buttonpane button:contains('Proceed')").button("disable");
    } else {
        $("#yearsTotalCashflow").css("color", "green");
        $(".ui-dialog-buttonpane button:contains('Proceed')").button("enable");
    }

}

function fundCashflowDialogSelector() {
    val = $('input[name=cashflowModel]:checked').val();
    console.debug("fundCashflowDialogSelector: val=" + val);
    if (val == 'standard') {
        $("#dialogCFStandard").css("display","block");
        $("#dialogCFCustom").css("display","none");
    } else {
        $("#dialogCFStandard").css("display","none");
        $("#dialogCFCustom").css("display","block");
    }

}

function addFundNoteValidation() {
    var allGood = true;


    if ($('#datepickerFundNote').val() == "") {
        $('#fundNoteDateHeader').css('color', 'red');
        allGood = false;
    } else
        $('#fundNoteDateHeader').css('color', 'black');

    if ($('#fundNote').val() == "") {
        $('#fundNoteNoteHeader').css('color', 'red');
        allGood = false;
    } else
        $('#fundNoteNoteHeader').css('color', 'black');


    return allGood;
}
function addOrUpdateFund() {
    var form = document.getElementById("updateFund");

    if (addFundValidation())
        form.submit();
    else
        angelJSerror("Please enter all required fields. Missing fields are red.")

}   // addOrUpdateFund

function addFundValidation() {
    var allGood = true;


    if ($('#fundName').val() == "") {
        $('#fundNameLabel').css('color', 'red');
        allGood = false;
    } else
        $('#fundNameLabel').css('color', 'black');

    if ($('#fundDescription').val() == "") {
        $('#fundDescriptionLabel').css('color', 'red');
        allGood = false;
    } else
        $('#fundDescriptionLabel').css('color', 'black');


    return allGood;
}

function showFundNoteAddForm() {
    var div = document.getElementById("addFundNoteDiv");
    var date = new Date();
    var options = {timeZone: 'UTC'};

    $("#datepickerFundNote").datepicker("setDate",date.toLocaleDateString('en-US',options));

    div.style.display = "block";

}
/*
 * return true if this fund matches the filter otherwise false
 */
function checkFundsFilter(fund) {
    if (FundsView.filter == "all" ||
        (FundsView.filter == "carry" && fund.id in FundInvestments.carryMap) ||
        (FundsView.filter == "invest" && fund.id in FundInvestments.commitMap) ||
        FundsView.filter == fund.status)
        return(true);
    else
        return(false);
}

function showTracker(investorID) {

    console.debug("showTracker: investorID is: " + investorID);

    InvestorID = investorID;    // keep investor ID around as global

    showTrackerSorted(investorID, 0);

}

function showTrackerSorted(investorID, cell) {

    var contentSummaryDiv = document.getElementById('contentSummary');


    var sort = "date";
    var property = "payDate";
    var direction = "DESC";

    /*
     * if cell is 0, this is the first call when the page is loaded, so the default values above will apply. otherwise...
     */
    if (cell != 0) {
        console.debug("showTrackerSorted investorID = " + investorID + " sort=" + cell.getAttribute("mySort") + " prop=" + cell.getAttribute("myProp") + " cellid=" + cell.id);
        sort = cell.getAttribute("mySort");
        property = cell.getAttribute("myProp");
        direction = sortToggle(cell);

        FundsView.saveSort("funds", sort,property, direction);

    } else {
        console.debug("showTrackerSorted first call (investorID,# funds) = " + angelParens(investorID, TrackerList.length));

        AngelView.restore();
        sort = AngelView.sortCol["funds"];
        property = AngelView.sortProp["funds"];
        direction = AngelView.sortDir["funds"];

    }


    $("#trackerTable tbody tr").remove();     // remove existing sort

    TrackerList.sort(function(a,b) {
        return trackerSorter(a, b, sort, direction, property);

    });

    // console.debug("showTrackersorted: list: " + JSON.stringify(Trackerlist));

    var tr;
    var count = 0;
    for (i=0; i<TrackerList.length; i++) {
        tr = TrackerList[i];
        if (tr != null) {       // only happens if we have a bug!
            if (AngelView.trackerFilter == "all" || tr.type == AngelView.trackerFilter) {
                addRowTrackerTable(tr, count);
                count++;
            }
        }
    }

}   // showTrackerSorted

function addRowTrackerTable(tr, index) {
    var table = document.getElementById("trackerTable");
    var body = table.tBodies[0];
    var row = body.insertRow(-1);
    var co, person, id;
    var name;
    var notes;
    var profile = "";
    var nextLive= "";

    row.style.backgroundColor = RowBackgrounds[index % RowBackgrounds.length];

    var trackerColMap = {"name":0, "type":1, "date":2, "next":3, "frequency": 4, "status":5,"notes":6};

    if (tr.status.substring(0,4) == "live") {
        // row.style.backgroundColor = "#eab7ad";
        row.style.color = "green";
    }

    // console.debug("addRowTrackerTable: TrackerList = " + JSON.stringify(TrackerList));

    if (tr.trackerDate == null || tr.trackerDate == "")
        tr.trackerDate = "N/A";

    if (tr.type == "person") {
        id = tr.personID;
        name = trackedPersonIDtoname(id);
        profile = "/userProfile?id=" + id;
    } else if (tr.type == "company") {
        id = Number(tr.companyID);
        name = trackedCompanyIDtoname(id);
        profile = "/companyPage?id=" + id;
    } else {
        name = "oops, not a person or company??";
    }

    nextLive = trackerNextLive(tr);
    if (tr.nextContactDate == null || tr.nextContactDate == "")
        tr.nextContactDate = trackerNextLive(tr);


    row.insertCell(trackerColMap["name"]).innerHTML = "<div id=optionsDiv" + tr.id + " class='optionsClass" + tr.id + " optionsArrow'" +
        " onclick='showTrackerOptions(this," + tr.id + ")' class='optionsArrow' style='float:left'></div> &nbsp;" +
        "<a href=" + profile + ">" + name  + "</a>";
    row.insertCell(trackerColMap["type"]).innerHTML = tr.type;
    row.insertCell(trackerColMap["date"]).innerHTML = tr.trackerDate;
    row.insertCell(trackerColMap["next"]).innerHTML = tr.nextContactDate;
    row.insertCell(trackerColMap["frequency"]).innerHTML = tr.frequency;
    row.insertCell(trackerColMap["status"]).innerHTML =  tr.status;
    row.cells[trackerColMap["status"]].addEventListener("click", function() {addTrackerStatusUpdate(tr.id)});
    row.cells[trackerColMap["status"]].addEventListener("mouseover", function() {row.cells[trackerColMap["status"]].style.border = "2px solid blue";});
    row.cells[trackerColMap["status"]].addEventListener("mouseout", function() {row.cells[trackerColMap["status"]].style.border = "0px";row.cells[trackerColMap["status"]].style.borderRight = "2px solid gray";});

    row.cells[trackerColMap["next"]].addEventListener("click", function() {addTrackerNextUpdate(tr.id)});
    row.cells[trackerColMap["next"]].addEventListener("mouseover", function() {row.cells[trackerColMap["next"]].style.border = "2px solid blue";});
    row.cells[trackerColMap["next"]].addEventListener("mouseout", function() {row.cells[trackerColMap["next"]].style.border = "0px";row.cells[trackerColMap["next"]].style.borderRight = "2px solid gray";});



    notes = removeNewlinesAndSlashes(tr.notes);
    row.insertCell(trackerColMap["notes"]).innerHTML = notes;

}   //addRowTrackerTable

function addTrackerStatusUpdate(id) {
    console.debug("addTrackerStatusUpdate: id: " + id);


    var trackerName = document.getElementById('trackerStatusUpdateName');
    var tr = TrackerMap[id.toString()];
    var trackedName;
    // console.debug("addTrackerStatusUpdate: " + JSON.stringify(tr));

    if (tr.type == "company")
        trackedName = trackedCompanyIDtoname(tr.companyID);
    else
        trackedName = trackedPersonIDtoname(tr.personID);

    // console.debug("addTrackerStatusUpdate: status " + tr.status);
    // $("input[type='radio']").attr("checked",false);
    if (tr.status == "live") {
        $("#trackerStatusLive").prop("checked",true);
    } else if (tr.status == "live:emailed") {
        $("#trackerStatusLiveEmailed").prop("checked",true);
    } else if (tr.status == "wait") {
        $("#trackerStatusWait").prop("checked",true);
    }
    // $("#trackerStatusLive").prop("checked",true);
    trackerName.innerHTML = "<span style='color:blue'>" + trackedName + "</span>";


    $("#dialogTrackerStatusUpdate").dialog({
        resizable: false,
        height: 300,
        width: 550,
        modal: true,
        buttons: {
            Proceed: function() {
                proceed(tr);
                $(this).dialog("close");
            },
            Cancel: function() {
                $(this).dialog("close");
            }
        }
    });

    function proceed(tr){

        var status = $('input[name=trackerStatus]:checked').val();
        var today = new Date();
        var dt = today.getFullYear() + "-" + twoDigitMonth(today.getMonth()+1) + "-" + twoDigitDate(today.getDate());
        // console.debug("***addTrackerStatusUpdate: proceed: dt: " + dt);

        if (tr.status == status)    // no change
            return;

        tr.status = status;
        tr.lastStatusUpdate = dt;  // today!

        $.get("tracker?id=" + tr.id + "&ajaxTrackerStatusUpdate="+status,
            function(data,status) {

                $(document).ajaxStop(function() {
                    showTracker(InvestorID);

                        // console.debug("trackerStatusUpdate return: " + JSON.stringify(data));
                    // $.notify("Status updated", 'success');
                });
            });
    }

}       // addTrackerStatusUpdate

function addTrackerNextUpdate(id) {
    console.debug("addTrackerNextUpdate: id: " + id);


    var trackerName = document.getElementById('trackerNextUpdateName');
    var tr = TrackerMap[id.toString()];
    var trackedName;
    // console.debug("addTrackerStatusUpdate: " + JSON.stringify(tr));

    if (tr.type == "company")
        trackedName = trackedCompanyIDtoname(tr.companyID);
    else
        trackedName = trackedPersonIDtoname(tr.personID);


    $("#datepickerTra").datepicker();
    var options = {timeZone: 'UTC'};
    var date;

    if (tr.nextContactDate != null && tr.nextContactDate != "") {
        date = new Date(tr.nextContactDate);
    } else {
        date = new Date();
    }
    $("#datepickerTra").datepicker("setDate",date.toLocaleDateString('en-US',options));
    //$("#trackerNextDate").html(tr.nextContactDate);

    trackerName.innerHTML = "<span style='color:blue'>" + trackedName + "</span>";


    $("#dialogTrackerNextUpdate").dialog({
        resizable: false,
        height: 320,
        width: 550,
        modal: true,
        buttons: {
            Proceed: function() {
                proceed(tr);
                $(this).dialog("close");
            },
            Cancel: function() {
                $(this).dialog("close");
            }
        }
    });

    function proceed(tr){

        var nextDate = $('#datepickerTra').val();
        var date = new Date(nextDate);
        var dt = date.getFullYear() + "-" + twoDigitMonth(date.getMonth()+1) + "-" + twoDigitDate(date.getDate());
        var makeWait;

        console.debug("***addTrackerNextUpdate: proceed: : " + nextDate);

        if (tr.nextContactDate == dt)    // no change
            return;

        // console.debug("-----addTrackerNextUpdate: (tr.next, nextDate): " + angelParens(tr.nextContactDate, nextDate));

        tr.nextContactDate = dt;

        if ($('#trackerMakeWait').prop("checked") == true) {
            makeWait = "&ajaxTrackerMakeWait";
            tr.status = "wait";
        } else
            makeWait = "";

        $.get("tracker?id=" + tr.id + "&ajaxTrackerNextUpdate="+dt+makeWait,
            function(data,status) {

                $(document).ajaxStop(function() {
                    showTracker(InvestorID);

                    // console.debug("+++trackerNextUpdate return: " + JSON.stringify(data));
                    // $.notify("Status updated", 'success');
                });
            });
    }

}       // addTrackerNextUpdate

function trackerQuickNext(time) {

    var options = {timeZone: 'UTC'};

    var currentDate = $("#datepickerTra").val();
    var date = new Date(currentDate);
    var newDate;

    if (time == "week") {
        date.setDate(date.getDate() + 7);
    } else if (time == "month") {
        date.setDate(date.getDate() + 31);
    } else if (time == "quarter") {
        var monthIndex = date.getMonth();
        var year = date.getFullYear();
        var quarter = returnQuarter(monthIndex+1);
        if (quarter == 4)
            newDate = (year+1) + "-01-01";
        else
            newDate = year + "-" + quarterStartDate(quarter+1);  // next month is + 1 but add another since index starts at 0
        date = new Date(newDate);
    }

    $("#datepickerTra").datepicker("setDate",date.toLocaleDateString('en-US',options));

}

function trackerNextLive(tr) {
    var status = tr.status;

    var date = new Date();
    var monthIndex = date.getMonth();
    var year = date.getFullYear();
    var quarter = returnQuarter(monthIndex);

    if ((status.substring(0,4) == "live") || (tr.trackerDate != tr.lastStatusUpdate)) {
        if (tr.frequency == "monthly") {
            if (monthIndex == 11)
                return((year+1) + "-01-01");
            else {
                var month = twoDigitMonth(monthIndex+2); // next month is + 1 but add another since index starts at 0

                return(year + "-" + month + "-01");
            }
        } else if (tr.frequency == "quarterly") {
            if (quarter == 4)
                return((year+1) + "-01-01");
            else
                return(year + "-" + quarterStartDate(quarter+1));  // next month is + 1 but add another since index starts at 0
        } else if (tr.frequency == "yearly") {
            return((year+1) + "-" + twoDigitMonth(monthIndex+1) + "-01");
        }

    } else {
        if (tr.start == "Next Month") {
            if (monthIndex == 11)
                return((year+1) + "-01-01");
            else {
                var month = twoDigitMonth(monthIndex+2); // next month is + 1 but add another since index starts at 0

                return(year + "-" + month + "-01");
            }
        } else if (tr.start == "Next Quarter") {
            if (quarter == 4)
                return((year+1) + "-01-01");
            else
                return(year + "-" + quarterStartDate(quarter+1));  // next month is + 1 but add another since index starts at 0
        }
    }
    return("soon");
}

function quarterStartDate(quarter) {

    if (quarter == 1)
        return("01-01")
    else if (quarter == 2)
        return("04-01");
    else if (quarter == 3)
        return("07-01");
    else if (quarter == 4)
        return("10-01");

}

function twoDigitMonth(month) {

    if (month <= 9)
        return("0" + month)
    else
        return(month);
}

function twoDigitDate(date) {

    if (date <= 9)
        return("0" + date)
    else
        return(date);
}

function returnQuarter(month) {
    if (month >= 1 && month <= 3)
        return(1);
    else if (month > 3 && month <= 6)
        return(2);
    else if (month > 6 && month <= 9)
        return(3);
    else if (month > 9 && month <= 12)
        return(4);

}
function trackedPersonIDtoname(id) {
    var entry;

    for (var i=0; i < TrackedPeople.length; i++) {
        entry = TrackedPeople[i];
        if ("personID" in entry && entry.personID == id)
            return(entry.name);
    }
    return("person's name not found");
}
function trackedCompanyIDtoname(id) {
    var entry;

    for (var i=0; i < TrackedCompanies.length; i++) {
        entry = TrackedCompanies[i];
        if ("companyID" in entry && entry.companyID == id)
            return(entry.name);
    }
    return("company name not found");
}

function trackerChooser(element, choice) {
    console.debug("companyListChooser:" + choice);
    /*
     * first reset links
     */
    $("#trackerAllFilter").css("color","#337ab7");
    $("#trackerCompanyFilter").css("color","#337ab7");
    $("#trackerPeopleFilter").css("color","#337ab7");

    /*
     * make selected link black (unless we are not called from html) and display correct format
     */
    if (element != null)
        $(element).css("color","black");

    CurrentView.view.subDisplay = choice;
    if (choice == "all") {
        AngelView.trackerFilter = "all";
    } else if (choice == "companies") {
        AngelView.trackerFilter = "company";
    } if (choice == "people") {
        AngelView.trackerFilter = "person";
    }
    showTracker(InvestorID);
}

function showFunds(investorID) {

    console.debug("showFunds: investorID is: " + investorID);

    InvestorID = investorID;    // keep investor ID around as global

    showFundsSorted(investorID, 0);

}

function showFundsSorted(investorID, cell) {

    var contentSummaryDiv = document.getElementById('contentSummary');


    var sort = "date";
    var property = "payDate";
    var direction = "DESC";

    /*
     * if cell is 0, this is the first call when the page is loaded, so the default values above will apply. otherwise...
     */
    if (cell != 0) {
        console.debug("showFundsSorted investorID = " + investorID + " sort=" + cell.getAttribute("mySort") + " prop=" + cell.getAttribute("myProp") + " cellid=" + cell.id);
        sort = cell.getAttribute("mySort");
        property = cell.getAttribute("myProp");
        direction = sortToggle(cell);

        FundsView.saveSort("funds", sort,property, direction);

    } else {
        console.debug("showFundsSorted first call (investorID,# funds) = " + angelParens(investorID, Funds.list.length));

        FundsView.restore();
        sort = FundsView.sortCol["funds"];
        property = FundsView.sortProp["funds"];
        direction = FundsView.sortDir["funds"];

    }


    clearFundTable();     // remove existing sort

    Funds.list.sort(function(a,b) {
        return fundSorter(a, b, sort, direction, property);

    });

    // console.debug("showfundssorted: carrymap: " + JSON.stringify(FundInvestments.carryMap));

    var co;
    var count = 0;
    for (i=0; i<Funds.list.length; i++) {
        fu = Funds.list[i];
        // console.debug("show funds sorted: (fund,filter) " + angelParens(fu.name, FundsView.filter));
        if (fu != null) {       // only happens if we have a bug!
            if (checkFundsFilter(fu)) {
                addRowFundsTable(fu, count);
                count++;
            }
        }
    }

}   // showFundsSorted

function addRowFundsTable(fund, index) {
    var table = document.getElementById("fundsTable");
    var body = table.tBodies[0];
    var row = body.insertRow(-1);
    var notes;
    var haveCarry = "";

    row.style.backgroundColor = RowBackgrounds[index % RowBackgrounds.length];

    var fundsColMap = {"fund":0, "date":1,"commit":2, "paid": 3, "left":4,"returned":5,"roi":6, "notes":7};

    if (fund.id in FundInvestments.commitMap)
        fundCommit = FundInvestments.commitMap[fund.id];    // investment entry corresponding to commit
    else
        fundCommit = FundInvestments.carryMap[fund.id];    // only got a carry

    if (fund.id in FundInvestments.carryMap)
        haveCarry = "*";

    // console.debug("addRowFundsTable: FI.commitMapx = " + JSON.stringify(FundInvestments.commitMap));
    var paidPercent;
    var committed = Number(fundCommit.investAmount);
    var paid = Number(Funds.paidInCapital(fund.id));
    var left = committed - paid;
    if (committed != 0)
        paidPercent = Number(paid*100/committed).toFixed(0);
    else
        paidPercent = Number(0).toFixed(0);
    
    var returned = parseInt(Funds.returned(fund.id, FundPayouts));
    var roi;
    if (paid != 0)
        roi = (returned-paid)/paid * 100;
    else
        roi = 0;

    row.insertCell(fundsColMap["fund"]).innerHTML = "<div id=optionsDiv" + fundCommit.id + " class='optionsClass" + fundCommit.id + " optionsArrow'" +
        " onclick='showFundOptions(this," + fundCommit.id + ")' class='optionsArrow' style='float:left'></div> &nbsp;" +
        "<a href='/fundPage?id=" + fund.id + "'>" + fund.name + haveCarry + "</a>";
    row.insertCell(fundsColMap["date"]).innerHTML = fundCommit.invDate;
    row.insertCell(fundsColMap["commit"]).innerHTML = "$" + committed.toLocaleString();
    row.insertCell(fundsColMap["paid"]).innerHTML = "$" + paid.toLocaleString() + " (" + paidPercent + "%)";
    row.insertCell(fundsColMap["left"]).innerHTML =  "$" + left.toLocaleString();
    row.insertCell(fundsColMap["returned"]).innerHTML = "$" + returned.toLocaleString();
    if (fund.id in FundInvestments.carryMap)
        row.insertCell(fundsColMap["roi"]).innerHTML = "n/a";               // carry has no ROI!
    else
        row.insertCell(fundsColMap["roi"]).innerHTML = roi.toFixed(1) + "%";

    notes = removeNewlinesAndSlashes(fundCommit.notes);
    row.insertCell(fundsColMap["notes"]).innerHTML = notes;

}   //addRowFundsTable

function clearFundTable() {

    $("#fundsTable tbody tr").remove();
}

function addOrUpdateFundCommit() {
    var form = document.getElementById("addFundForm");
    var comm = document.getElementById("fundCommitAmount");
    var action = $('#fundAction').val();
    var fundID = $('#fund').val();


    /*
     * the numbers are formatted thanks to the jquery number formatter so we need to grab the number value to submit
     */
    comm.value =  $('#fundCommitAmount').val();


    if ((fundID in FundInvestments.commitMap || fundID in FundInvestments.carryMap ) && action == "addFund")
        angelJSerror("You have already invested in that fund.");
    else {

        if (addFundCommitValidation()) {
            console.debug("addorupdatefundcommit: validation passed.");
            form.submit();

        } else
            angelJSerror("Please enter all required fields. Missing fields are red.")
    }

}
function addFundCommitValidation() {
    var allGood = true;

    if ($('#fundCommitAmount').val() == 0 && $('#hasCarry').prop("checked") == false) {
        $('#fundAddCommitHeader').css('color', 'red');
        allGood = false;
    } else
        $('#fundAddCommitHeader').css('color', 'black');


    if ($('#datepickerFundAdd').val() == "") {
        $('#fundAddDateHeader').css('color', 'red');
        allGood = false;
    } else
        $('#fundAddDateHeader').css('color', 'black');

    if ($('#fund').val() == "") {
        $('#fundAddFundHeader').css('color', 'red');
        allGood = false;
    } else
        $('#fundAddFundHeader').css('color', 'black');


    return allGood;
}
function clearAddFundInvestmentForm() {

    var inv = 0;

    console.debug("clearAddFundInvestmentForm");


    var addDiv = document.getElementById('addInvestmentDiv');
    var date = new Date();
    var options = {timeZone: 'UTC'};
    document.getElementById("newFundInvestmentButton").innerHTML = "Add New Investment";


    $("#fund").select2("val", null);

    $("#datepickerFundAdd").datepicker("setDate",date.toLocaleDateString('en-US',options));


    $("#fundKind").val("vc");
    $("#fundCommitAmount").val(0);
    $("#fundNotes").val("");

    $("#wireBankName").val("");
    $("#wireBankAddress").val("");
    $("#wireBankRouting").val("");
    $("#wireBankSWIFT").val("");
    $("#wireBankAcct").val("");
    $("#wireFCName").val("");
    $("#wireFCAcct").val("");


    $('#invWiringDetails').css('display','none');
    $('#enterWiringText').css('display','block');

}
function clearAddFundPayoutForm(fund) {

    var inv = 0;

    console.debug("clearAddFundPayoutForm");


    var date = new Date();
    var options = {timeZone: 'UTC'};
    document.getElementById("newFundInvestmentButton").innerHTML = "Add New Investment";


    $("#fund").select2("val", null);

    $("#datepickerPayfund").datepicker("setDate",date.toLocaleDateString('en-US',options));



}
function showFundPayouts(investorID) {

    console.debug("showFundPayouts: investorID is: " + investorID);

    InvestorID = investorID;    // keep investor ID around as global

    showFundPayoutsSorted(investorID, 0);



}
function showFundPayoutsSorted(investorID, cell) {

    var sort = "date";
    var property = "payDate";
    var direction = "DESC";

    /*
     * if cell is 0, this is the first call when the page is loaded, so the default values above will apply. otherwise...
     */
    if (cell != 0) {
        console.debug("showFundPayoutsSorted investorID = " + investorID + " sort=" + cell.getAttribute("mySort") + " prop=" + cell.getAttribute("myProp") + " cellid=" + cell.id);
        sort = cell.getAttribute("mySort");
        property = cell.getAttribute("myProp");
        direction = sortToggle(cell);

        FundsView.saveSort("fundPayouts", sort,property, direction);


    } else {
        console.debug("showFundPayoutsSorted first call investorID = " + investorID);

        FundsView.restore();
        sort = FundsView.sortCol["fundpayouts"];
        property = FundsView.sortProp["fundpayouts"];
        direction = FundsView.sortDir["fundpayouts"];

    }


    clearPayoutTable("fund");     // remove existing sort

    FundPayouts.list.sort(function(a,b) {
        return investmentSorter(a, b, sort, direction, property);

    });

    var co;
    var count = 0;
    for (i=0; i<FundPayouts.list.length; i++) {
        fund = Funds.fund(FundPayouts.list[i].fundID);
        if (fund != null) {       // only happens if we have a bug!
            if (checkFundsFilter(fund)) {
                addRowFundPayoutTable(FundPayouts.list[i], count);
                count++;
            }
        }
    }

}
function addRowFundPayoutTable(po, index) {
    var table = document.getElementById("fundPayoutsTable");
    var body = table.tBodies[0];
    var row = body.insertRow(-1);
    var notes;

    po.type = po.type.toLocaleLowerCase();

    row.style.backgroundColor = RowBackgrounds[index % RowBackgrounds.length];


    var i = -1;


    row.insertCell(i+1).innerHTML = "<div id=optionsDiv" + po.id + " class='optionsClass" + po.id + " optionsArrow'" +
        " onclick='showPayoutOptions(this," + po.id + ",\"fund\")' class='optionsArrow' style='float:left'></div> &nbsp;" +
        "<a href='/fundPage?id=" + po.fundID + "'>" + Funds.map[(po.fundID).toString()].name + "</a>";
    row.insertCell(i+2).innerHTML = po.payDate;
    row.insertCell(i+3).innerHTML = po.type;
    if (po.type == "cash")
        row.insertCell(i+4).innerHTML = "$" + Number(po.cash).toLocaleString();
    else
        row.insertCell(i+4).innerHTML = "";

    if (po.type == "stock") {       // ack make sure this is stored in lowercase later
        row.insertCell(i+5).innerHTML = Number(po.shares).toLocaleString();
        row.insertCell(i+6).innerHTML = po.symbol;
        row.insertCell(i+7).innerHTML = Number(po.sharePrice).toFixed(4);
        row.insertCell(i+8).innerHTML = "$" + Math.round((Number(po.shares)*Number(po.sharePrice))).toLocaleString();
    } else { // cash
        row.insertCell(i+5).innerHTML = "";
        row.insertCell(i+6).innerHTML = "";
        row.insertCell(i+7).innerHTML = "";
        row.insertCell(i+8).innerHTML = "$" + Number(po.cash).toLocaleString();

    }
    notes = removeNewlinesAndSlashes(po.notes);
    row.insertCell(i+9).innerHTML = notes;

}   //addRowFundPayoutTable


function addOrUpdatePayout(fund) {
    // console.debug("addOrUpdateFundPayout: " + fund);
    var form = document.getElementById(fund+"addPayoutForm");

    var po = document.getElementById(fund+"payoutCash");
    var shrs = document.getElementById(fund+"payoutShares");
    var id;
    var idLeft;
    var companyID = 1;

    //var val = document.getElementById("value");

    // console.debug("---Add Payout: payoutCash/type:" + $('#payoutCash').val() +"/" + $('#payoutType').val());


    po.value =  $('#'+fund+'payoutCash').val();
    shrs.value = $('#'+fund+'payoutShares').val();

    fundID = $('#payout'+fund).val();



    /*
     * the numbers are formatted thanks to the jquery number formatter so we need to grab the number value to submit
     */


    if (addPayoutValidation(fund)) {

        form.submit();
        // $.notify("Would be adding payout now", "success");       // debug


    } else
        angelJSerror("Please enter all required fields. Missing fields are red.")

}
function addPayoutValidation(fund) {
    var allGood = true;
    if ($('#'+fund+'payoutType').val() == "cash") {
        // console.debug("validate: payoutcash: " + $('#payoutCash').val());
        if ($('#'+fund+'payoutCash').val() == 0) {
            $('#'+fund+'payoutAmountHeader').css('color', 'red');
            allGood = false;
        } else
            $('#'+fund+'payoutAmountHeader').css('color', 'black');
    } else {
        if ($('#'+fund+'payoutShares').val() == 0) {
            $('#'+fund+'payoutAmountHeader').css('color', 'red');
            allGood = false;
        } else
            $('#'+fund+'payoutAmountHeader').css('color', 'black');
        if ($('#'+fund+'payoutPrice').val() == 0) {
            $('#'+fund+'payoutPriceHeader').css('color', 'red');
            allGood = false;
        } else
            $('#'+fund+'payoutPriceHeader').css('color', 'black');
    }


    if ($('#datepickerPay'+fund).val() == "") {
        $('#'+fund+'payoutDateHeader').css('color', 'red');
        allGood = false;
    } else
        $('#'+fund+'payoutDateHeader').css('color', 'black');

    if ($('#payout' + fund).val() == "") {
        $('#'+fund+'payoutFundHeader').css('color', 'red');
        allGood = false;
    } else
        $('#'+fund+'payoutFundHeader').css('color', 'black');

    if ($('#'+fund+'payoutType').val() == null) {
        $('#'+fund+'payoutTypeHeader').css('color', 'red');
        allGood = false;
    } else
        $('#'+fund+'payoutTypeHeader').css('color', 'black');

    return allGood;
}

function showFundPayments(investorID) {

    console.debug("showFundPayments: investorID is: " + investorID);

    InvestorID = investorID;    // keep investor ID around as global

    showFundPaymentsSorted(investorID, 0);

}

function showFundPaymentsSorted(investorID, cell) {


    var sort = "date";
    var property = "invDate";
    var direction = "DESC";

    /*
     * if cell is 0, this is a call from jscript otherwise an html call from column header
     */
    if (cell != 0) {
        console.debug("showFundPaymentsSorted html call investorID = " + investorID + " sort=" + cell.getAttribute("mySort") + " prop=" + cell.getAttribute("myProp") + " cellid=" + cell.id);
        sort = cell.getAttribute("mySort");
        property = cell.getAttribute("myProp");
        direction = sortToggle(cell);

        FundsView.saveSort("fundpayments", sort,property, direction);

    } else {

        FundsView.restore();
        if ("fundpayments" in FundsView.sortCol)
            sort = FundsView.sortCol["fundpayments"];
        if ("fundpayments" in FundsView.sortProp)
            property = FundsView.sortProp["fundpayments"];
        if ("fundpayments" in FundsView.sortDir)
            direction = FundsView.sortDir["fundpayments"];

        console.debug("showFundPaymentsSorted jscript call (investorID, sort) (prop, dir) = " + angelParens(investorID, sort) + angelParens(property, direction));
    }


    clearFundPaymentsTable();     // remove existing sort

    FundInvestments.list.sort(function(a,b) {
        return fundPaymentSorter(a, b, sort, direction, property);

    });

    var count = 0;
    for (i=0; i<FundInvestments.list.length; i++) {
        fpay = FundInvestments.list[i];
        // console.debug("showfundpayments sorted: (fpay type,filter) " + angelParens(fpay.type, FundsView.filter));
        if (fpay != null && fpay.type == "invest") {       // only add if this is an investment aka payment
            if (checkFundsFilter(fund)) {
                addRowFundPaymentsTable(fpay, count);
                count++;
            }
        }
    }

}   // showFundPaymentsSorted

function addRowFundPaymentsTable(fpay, index) {
    var table = document.getElementById("fundPaymentsTable");
    var body = table.tBodies[0];
    var row = body.insertRow(-1);
    var notes;

    row.style.backgroundColor = RowBackgrounds[index % RowBackgrounds.length];

    fundsColMap = {"fund":0, "date":1,"amount":2, "left":3, "notes":4};

    fundCommit = FundInvestments.commitMap[fpay.fundID];    // investment entry corresponding to commit
    //console.debug("addRowFundsTable: FI.commitMap = " + JSON.stringify(FundInvestments.commitMap));

    var committed = Number(fundCommit.investAmount);
    var paid = Number(Funds.paidInCapital(fpay.fundID));
    var investAmount = Number(fpay.investAmount);
    var left = committed - paid;

    fund = Funds.map[fpay.fundID];

    row.insertCell(fundsColMap["fund"]).innerHTML = "<div id=optionsDiv" + fpay.id + " class='optionsClass" + fpay.id + " optionsArrow'" +
        " onclick='showFundPaymentOptions(this," + fpay.id + ")' class='optionsArrow' style='float:left'></div> &nbsp;" +
        "<a href='/fundPage?id=" + fpay.fundID + "'>" + fund.name + "</a>";
    row.insertCell(fundsColMap["date"]).innerHTML = fpay.invDate;
    row.insertCell(fundsColMap["amount"]).innerHTML = "$" + investAmount.toLocaleString();
    row.insertCell(fundsColMap["left"]).innerHTML =  "$" + left.toLocaleString();

    notes = removeNewlinesAndSlashes(fpay.notes);
    row.insertCell(fundsColMap["notes"]).innerHTML = notes;

}   //addRowFundPaymentsTable

function clearFundPaymentsTable() {

    $("#fundPaymentsTable tbody tr").remove();
}

function addOrUpdateFundPayment() {
    var form = document.getElementById("fundaddPaymentForm");
    var comm = document.getElementById("fundpaymentAmount");
    var action = $('#fundpaymentAction').val();


    /*
     * the numbers are formatted thanks to the jquery number formatter so we need to grab the number value to submit
     */
    comm.value =  $('#fundpaymentAmount').val();

    if (addFundPaymentValidation()) {
        // console.debug("addorupdatefundpayment: validation passed.");
        form.submit();

    } else
        angelJSerror("Please enter all required fields. Missing fields are red.")


}
function addFundPaymentValidation() {
    var allGood = true;

    /*    if ($('#fundCommitAmount').val() == 0) {        // maybe this shouldn't be required (e.g. for YC)
     $('#fundAddCommitHeader').css('color', 'red');
     allGood = false;
     } else
     $('#fundAddCommitHeader').css('color', 'black');*/

    if ($('#datepickerFundPayment').val() == "") {
        $('#fundpaymentDateHeader').css('color', 'red');
        allGood = false;
    } else
        $('#fundpaymentDateHeader').css('color', 'black');

    if ($('#fundpaymentAmount').val() == 0) {
        $('#fundpaymentAmountHeader').css('color', 'red');
        allGood = false;
    } else
        $('#fundpaymentAmountHeader').css('color', 'black');

    if ($('#paymentfund').val() == "") {
        $('#fundpaymentFundHeader').css('color', 'red');
        allGood = false;
    } else
        $('#fundpaymentFundHeader').css('color', 'black');


    return allGood;
}

function cashflowChooser(element, choice) {

    // console.debug("cashflowChooser:" + choice);
    /*
     * first reset links
     */
    $("#actualCashFlowsChoice").css("color","#337ab7");
    $("#predictedCashFlowsChoice").css("color","#337ab7");

    /*
     * make selected link black (unless we are not called from html) and display correct format
     */
    if (element != null)
        $(element).css("color","black");

    CurrentView.view.subDisplay = choice;
    if (choice == "actuals") {
        $("#fundPredictionsContainer").css("display","none");
        $('#actualCashFlowsChoice').css("color","black");
        $("#fundActualsContainer").css("display","block");
        $('#includeCapitalPaymentsSpan').css("display","inline");
    } else if (choice == "predictions") {
        $('#predictedCashFlowsChoice').css("color","black");
        $("#fundPredictionsContainer").css("display","block");
        $("#fundActualsContainer").css("display","none");
        $('#includeCapitalPaymentsSpan').css("display","none");
    }
}

function clearFundPerformanceTables() {

    $("#fundPerformanceTable tbody tr").remove();
    $("#fundPerformanceTotalsTable tbody tr").remove();

}
function showFundPerformance(investorID) {

    console.debug("showFundPerformance: investorID is: " + investorID);

    InvestorID = investorID;    // keep investor ID around as global

    showFundPerformanceSorted(investorID, 0);

}

function showFundPerformanceSorted(investorID, cell) {


    var sort = "fund";
    var property = "id";
    var direction = "ASC";

    /*
     * if cell is 0, this is the first call when the page is loaded, so the default values above will apply. otherwise...
     */
    if (cell != 0) {
        console.debug("showFundPerformanceSorted investorID = " + investorID + " sort=" + cell.getAttribute("mySort") + " prop=" + cell.getAttribute("myProp") + " cellid=" + cell.id);
        sort = cell.getAttribute("mySort");
        property = cell.getAttribute("myProp");
        direction = sortToggle(cell);

        FundsView.saveSort("fundperformance", sort,property, direction);

    } else {
        console.debug("showFundPerformanceSorted first call (investorID,# funds Investments) = " + angelParens(investorID, FundInvestments.list.length));

        FundsView.restore();
        sort = FundsView.sortCol["fundperformance"];
        property = FundsView.sortProp["fundperformance"];
        direction = FundsView.sortDir["fundperformance"];

    }

    /*
     * first collect all the per-year data we need to evaluate performance
     */
    clearFundPerformanceTables();       // remove existing sort
    fundReturnsByYear.clearFunds();         // make sure we are starting clean
    FundPerformanceTotals.clear();

    for (i=0; i<Funds.list.length; i++) {
        fund = Funds.list[i];
        if (fund != null && fund.id in FundInvestments.commitMap)
            fundReturnsByYear.addFundActuals(fund);
    }

    Funds.list.sort(function(a,b) {
        return fundPerformanceSorter(a, b, sort, direction, property);

    });

    var co;
    var count = 0;
    for (i=0; i<Funds.list.length; i++) {
        fund = Funds.list[i];
        // console.debug("showfundperformance sorted: (fund type,filter) " + angelParens(fpay.type, FundsView.filter));
        /*
         * add the fund if it is found and it is an investment fund, not exclusively one with carry
         */
        if (fund != null && fund.id in FundInvestments.commitMap) {
            if (checkFundsFilter(fund)) {
                addRowFundPerformanceTable(fund, count);
                count++;
            }
        }
    }
    addRowFundPerformanceTotalsTable();

}   // showFundPerformanceSorted

function addRowFundPerformanceTable(fund, index) {
    var table = document.getElementById("fundPerformanceTable");
    var body = table.tBodies[0];
    var row = body.insertRow(-1);
    var fillerColor = "aliceblue";
    var irr = 0;

    row.style.backgroundColor = RowBackgrounds[index % RowBackgrounds.length];

    var fundsColMap = {"fund":0, "invested":1,"returned":2, "capacct":3, "prediction":4,
                       "filler1": 5, "actualirr":6, "actualtvpi":7,
                        "filler2": 8, "actualcatvpi":9,
                        "filler3": 10, "actualprtvpi":11};

    var fundCommit = FundInvestments.commitMap[fund.id];    // investment entry corresponding to commit

    //console.debug("addRowFundPerformanceTable: FI.commitMap = " + JSON.stringify(FundInvestments.commitMap));

    var paid = Number(Funds.paidInCapital(fund.id));
    var returned = parseInt(Funds.returned(fund.id, FundPayouts));

    FundPerformanceTotals.invested += paid;
    FundPerformanceTotals.returned += returned;

    // console.debug("addRowFundPerformanceTable: fund (paid,returned): " + fund.name + angelParens(paid, returned));

    row.insertCell(fundsColMap["fund"]).innerHTML = "<a href='/fundPage?id=" + fund.id + "'>" + fund.name  + "</a>";

    row.insertCell(fundsColMap["invested"]).innerHTML =  Math.round(paid).toLocaleString();
    row.insertCell(fundsColMap["returned"]).innerHTML = Math.round(returned).toLocaleString();

    var capAcct,predictedReturn;
    if ("capitalAccount" in fund && fund.capitalAccount > 0) {
        capAcct  = Math.round(Number(fund.capitalAccount));
    } else
        capAcct = 0;
    row.insertCell(fundsColMap["capacct"]).innerHTML = capAcct.toLocaleString();

    row.cells[fundsColMap["capacct"]].addEventListener("click", function() {addCashflowUpdate(fund.id,"valuation update")});
    row.cells[fundsColMap["capacct"]].addEventListener("mouseover", function() {row.cells[fundsColMap["capacct"]].style.border = "2px solid blue";});
    row.cells[fundsColMap["capacct"]].addEventListener("mouseout", function() {row.cells[fundsColMap["capacct"]].style.border = "0px";row.cells[fundsColMap["capacct"]].style.borderRight = "2px solid gray";});


    if ("predictedReturn" in fund && fund.predictedReturn > 0) {
        predictedReturn  = Math.round(Number(fund.predictedReturn));

    }  else
        predictedReturn = 0;

    FundPerformanceTotals.capAccount += capAcct;
    FundPerformanceTotals.predictedReturn += predictedReturn;

    row.insertCell(fundsColMap["prediction"]).innerHTML = predictedReturn.toLocaleString();

    row.cells[fundsColMap["prediction"]].addEventListener("click", function() {addCashflowUpdate(fund.id,"exit prediction")});
    row.cells[fundsColMap["prediction"]].addEventListener("mouseover", function() {row.cells[fundsColMap["prediction"]].style.border = "2px solid blue";});
    row.cells[fundsColMap["prediction"]].addEventListener("mouseout", function() {row.cells[fundsColMap["prediction"]].style.border = "0px";row.cells[fundsColMap["prediction"]].style.borderRight = "2px solid gray";});


    cell = row.insertCell(fundsColMap["filler1"]);
    cell.style.backgroundColor = fillerColor;
    cell.innerHTML = "";
    if (returned == 0  || returned < paid)
        row.insertCell(fundsColMap["actualirr"]).innerHTML = "n/a";
    else {
        irr = calcFundIRR(fund, 0);
        if (irr == "error")
            row.insertCell(fundsColMap["actualirr"]).innerHTML = "invalid";
        else
            row.insertCell(fundsColMap["actualirr"]).innerHTML = (irr*100).toFixed(1)+"%";
    }
    row.insertCell(fundsColMap["actualtvpi"]).innerHTML = (returned/paid).toFixed(2);


    cell = row.insertCell(fundsColMap["filler2"]);
    cell.style.backgroundColor = fillerColor;
    cell.innerHTML = "";

    row.insertCell(fundsColMap["actualcatvpi"]).innerHTML = ((returned+capAcct)/paid).toFixed(2);

    cell = row.insertCell(fundsColMap["filler3"]);
    cell.style.backgroundColor = fillerColor;
    cell.innerHTML = "";

    // console.debug("addRowFundPerformanceTable: weird! (paid,returned) predictedReturn" + angelParens(paid, returned) + predictedReturn);
    row.insertCell(fundsColMap["actualprtvpi"]).innerHTML = ((returned+predictedReturn)/paid).toFixed(2);

}   //addRowFundPerformanceTable

function addRowFundPerformanceTotalsTable() {
    var table = document.getElementById("fundPerformanceTotalsTable");
    var body = table.tBodies[0];
    var row = body.insertRow(-1);
    var fillerColor = "aliceblue";
    var irr = 0;



    var fundsColMap = {"fund":0, "invested":1,"returned":2, "capacct":3, "prediction":4,
        "filler1": 5, "actualirr":6, "actualtvpi":7,
        "filler2": 8, "actualcatvpi":9,
        "filler3": 10, "actualprtvpi":11};

    cell = row.insertCell(0);
    cell.innerHTML = "<hr>";
    cell.colSpan = 12;
    row = body.insertRow(-1);

    row.style.color = "blue";
    row.insertCell(fundsColMap["fund"]).innerHTML = "Totals";

    row.insertCell(fundsColMap["invested"]).innerHTML =  Math.round(FundPerformanceTotals.invested).toLocaleString();
    row.insertCell(fundsColMap["returned"]).innerHTML = Math.round(FundPerformanceTotals.returned).toLocaleString();
    row.insertCell(fundsColMap["capacct"]).innerHTML = Math.round(FundPerformanceTotals.capAccount).toLocaleString();
    row.insertCell(fundsColMap["prediction"]).innerHTML = Math.round(FundPerformanceTotals.predictedReturn).toLocaleString();
    cell = row.insertCell(fundsColMap["filler1"]);
    cell.style.backgroundColor = fillerColor;
    cell = row.insertCell(fundsColMap["actualirr"]);    // no total here
    cell.style.backgroundColor = fillerColor;
    row.insertCell(fundsColMap["actualtvpi"]).innerHTML = (FundPerformanceTotals.returned/FundPerformanceTotals.invested).toFixed(2);
    cell = row.insertCell(fundsColMap["filler2"]);
    cell.style.backgroundColor = fillerColor;
    row.insertCell(fundsColMap["actualcatvpi"]).innerHTML = ((FundPerformanceTotals.returned+FundPerformanceTotals.capAccount)/FundPerformanceTotals.invested).toFixed(2);
    cell = row.insertCell(fundsColMap["filler3"]);
    cell.style.backgroundColor = fillerColor;
    row.insertCell(fundsColMap["actualprtvpi"]).innerHTML = ((FundPerformanceTotals.returned+FundPerformanceTotals.predictedReturn)/FundPerformanceTotals.invested).toFixed(2);


}   //addRowFundPerformanceTotalsTable

/*
 * calculate and return the IRR for the input fund. if future is not zero, then use the fund's model to
 * predict future cashflows and include those as well (with any cash payments left which will be divided
 * evenly over the next 3 years).
 */
function calcFundIRR(fund, future) {

    var values = [];
    var irr = .20;

    var years = fundReturnsByYear.funds[fund.id];
    // console.debug("calcFundIRR: fund - years: " + fund.name + " - " + JSON.stringify(years));
    var i = 0;
    var total = 0;
    for (var year in years) {
        values[i]  = years[year].return_amount - years[year].invest_amount;
        total += values[i];
        i++;
    }
    // console.debug("calcFundIRRy: (total,values): " + angelParens(total,JSON.stringify(values)));
    if (total < 0)
        return("error");

    if (future == 0) {      // actuals only
        irr = IRR(values,.1);
        // console.debug("calcFundIRR: irr: " + irr);
    }
    return(irr);
}

function clearCashflowTables() {

    $("#fundActualsTable tbody tr").remove();
    $("#fundActualsTotalsTable tbody tr").remove();
    $("#fundPredictionsTable tbody tr").remove();
    $("#fundPredictionsTotalsTable tbody tr").remove();

}
function showFundCashflows(investorID) {

    clearCashflowTables();  // clear previous

    var pastTable = document.getElementById("fundActualsTable");
    var pastBody = pastTable.tBodies[0];
    var pastRow = pastBody.insertRow(-1);

    var futureTable = document.getElementById("fundPredictionsTable");
    var futureBody = futureTable.tBodies[0];
    var futureRow = futureBody.insertRow(-1);

    var startYear = CurrentYear - 5;

    var fundsColMap = {"fund":0, "previous":1};

    console.debug("showFundCashflows: investorID is: " + investorID);

    fundReturnsByYear.clearFunds();


    Funds.list.sort(function(a,b) {
        return fundSorter(a, b, "fund", "ASC", "id");

    });

    var numberFunds = 0;
    for (i=0; i<Funds.list.length; i++) {
        fund = Funds.list[i];
        if (!checkFundsFilter(fund))
            continue;
        numberFunds++;

        futureRow.style.backgroundColor = RowBackgrounds[i % RowBackgrounds.length];


        fundReturnsByYear.addFundActuals(fund);
        insertFCFPast(fund, pastRow, pastBody, i);

        fundReturnsByYear.addFundFutures(fund);
        insertFCFFuture(fund, futureRow, futureBody, i);

        pastRow = pastBody.insertRow(-1);
        futureRow = futureBody.insertRow(-1);
    }

    /*
     * this will make the totals line appear next to the funds -- DEPRECATED. Showing them all now.

    if (numberFunds < 30) {
        $('#fundActualsTableDiv').outerHeight(numberFunds*20);
        $('#fundPredictionsTableDiv').outerHeight(numberFunds*20);
    } else {
        $('#fundActualsTableDiv').outerHeight(600);
        $('#fundPredictionsTableDiv').outerHeight(600);
    }
    */

    // console.debug("showfundcashflows w - END 2017: return_amount " +  fundReturnsByYear.fundYears[2017].return_amount);
    insertFCFPastTotals();
    insertFCFFutureTotals();

    hideGraphics();

}
/*
 * Fund Cash Flow Past (FCFPast) currently doing 5 years ago (6 years incl this year)
 *
 * Naming convention note (because it's kinda ugly).
 *        <name>Carr refers to a fund which is carry only
 *        <name>Inv refers to a fund to which investment capital has been committed
 *        <name>Invest refers to funds which have been invested in an xInv fund.
 */
function insertFCFPast(fund, row, body, index) {
    var amount = 0, invest = 0, total = 0, totalInvest = 0, previous = 0, previousInvest = 0;
    var years;
    var includePayments = $('input[name="includePayments"]').is(':checked');
    var startYear = CurrentYear - FundTotalsInv.pastYears;
    var haveCarry = "";

    /*
     * A fund is either in an investment fund or a carry only fund. Note must check the commit funds first for
     * the case where a fund is both (it will only be in FundTotalsInv)
     */
    if (fund.id in FundInvestments.commitMap)
        years = FundTotalsInv.funds[fund.id];
    else {
        years = FundTotalsCarry.funds[fund.id];
        haveCarry = "*";
    }


    row.insertCell(0).innerHTML = "<a href='/fundPage?id=" + fund.id + "'>" + fund.name  + haveCarry + "</a>";
    if (includePayments) {
        var nextRow = body.insertRow(-1);
        nextRow.insertCell(0).innerHTML = "";
        row.style.backgroundColor = RowBackgrounds[index % RowBackgrounds.length];
        nextRow.style.backgroundColor = RowBackgrounds[index % RowBackgrounds.length];
    } else
        row.style.backgroundColor = RowBackgrounds[index % RowBackgrounds.length];

    for (var y in years) {
        if (y < startYear) {
            previous += Math.round(years[y].return_amount/1000);
            previousInvest += Math.round(years[y].invest_amount/1000);
            total = previous;
            totalInvest = previousInvest;
        }
    }
    row.insertCell(1).innerHTML = previous.toLocaleString();          // previous amount
    if (includePayments)
        nextRow.insertCell(1).innerHTML =
            "<span class='fundPaymentSpan'>(" + previousInvest.toLocaleString() + ")</span>";

    /*
     * j is the index of the cell in the table, and the first cell [0] is fhe fund name next cell [1] previous years
     */

    for (j=2; j <= FundTotalsInv.pastYears+2; j++) {
        amount = 0;
        invest = 0;
        year = startYear+j-2;

        if (years.hasOwnProperty(year)) {
            amount = Math.round(years[year].return_amount/1000);
            invest = Math.round(years[year].invest_amount/1000);
        }

        row.insertCell(j).innerHTML = amount.toLocaleString();
        if (includePayments)
            nextRow.insertCell(j).innerHTML = "<span class='fundPaymentSpan'>(" + invest.toLocaleString() + ")</span>";
        total += amount;
        totalInvest += invest;

    }

    // total
    cell = row.insertCell(FundTotalsInv.pastYears+3);
    cell.innerHTML = total.toLocaleString();
    cell.style.color = "blue"

    if (includePayments)
        nextRow.insertCell(FundTotalsInv.pastYears+3).innerHTML = "<span class='fundPaymentSpan'>(" + totalInvest.toLocaleString() +")</span>";
}

/*
 * Fund Cash Flow Past (FCFPast) currently doing 5 years ago (6 years incl this year)
 */
function insertFCFPastTotals() {
    var totalsTable = document.getElementById("fundActualsTotalsTable");
    var body = totalsTable.tBodies[0];
    var yearsInv, yearsCarr;

    var rowInv;
    var rowCarr;
    var row;


    var amount, amountInv, investAmount, total = 0, totalInvest = 0, totalInv = 0, totalCarr = 0,
        previous = 0, previousInv = 0, previousCarr = 0, previousInvest = 0;
    var includePayments = $('input[name="includePayments"]').is(':checked');
    var startYear = CurrentYear - FundTotalsCarry.pastYears;

    rowInv = body.insertRow(-1);
    cell = rowInv.insertCell(0);
    cell.innerHTML = "<hr>";
    cell.colSpan = 14;
    rowInv = body.insertRow(-1);

    rowCarr = body.insertRow(-1);
    row = body.insertRow(-1);

    cell = row.insertCell(0);
    cell.innerHTML = "<hr>";
    cell.colSpan = 14;
    row = body.insertRow(-1);



    row.style.color = "blue";
    rowInv.style.color = "blue";
    rowCarr.style.color = "blue";

    if (includePayments) {
        var nextRow = body.insertRow(-1);
        nextRow.insertCell(0).innerHTML = "";
    }

    yearsCarr = FundTotalsCarry.years;
    yearsInv = FundTotalsInv.years;

    for (var y in yearsCarr) {
        if (y < startYear) {
            if (yearsCarr.hasOwnProperty(y)) {
                previousCarr += Math.round(yearsCarr[y].return_amount/1000);
            }
        }
    }
    for (var y in yearsInv) {
        if (y < startYear) {
            if (yearsInv.hasOwnProperty(y)) {
                previousInv += Math.round(yearsInv[y].return_amount/1000);
            }
            previousInvest += Math.round(yearsInv[y].invest_amount/1000);
            // console.debug("insertFCFPastTotals: startYear ... " + startYear + " " + angelParens(y, Math.round(yearsInv[y].invest_amount/1000)));
        }
    }
    previous = previousCarr + previousInv;


    rowInv.insertCell(0).innerHTML = "Totals Commit";
    rowInv.insertCell(1).innerHTML = previousInv.toLocaleString();

    rowCarr.insertCell(0).innerHTML = "Totals Carry Only";
    rowCarr.insertCell(1).innerHTML = previousCarr.toLocaleString();

    row.insertCell(0).innerHTML = "Totals";
    row.insertCell(1).innerHTML = previous.toLocaleString();

    if (includePayments) {
        nextRow.insertCell(1).innerHTML = "<span class='fundPaymentSpan'>(" +
            previousInvest.toLocaleString() + ")</span>";
    }

    /*
     * j is the index of the cell in the table, and the first cell [0] is fhe fund name next cell [1] previous years
     */
    for (j=2; j <= FundTotalsCarry.pastYears+2; j++) {
        year = startYear+j-2;


        amount = 0;
        if (yearsCarr.hasOwnProperty(year)) {
            amount = Math.round(yearsCarr[year].return_amount/1000);
            totalCarr += amount;

        }
        rowCarr.insertCell(j).innerHTML = amount.toLocaleString();

        amountInv = 0;
        if (yearsInv.hasOwnProperty(year)) {
            amountInv = Math.round(yearsInv[year].return_amount/1000)
            totalInv += amountInv;
            amount += amountInv;
            investAmount = Math.round(yearsInv[year].invest_amount/1000);
        }
        rowInv.insertCell(j).innerHTML = amountInv.toLocaleString();

        total += amount;
        totalInvest += investAmount;
        row.insertCell(j).innerHTML = amount.toLocaleString();
        if (includePayments)
            nextRow.insertCell(j).innerHTML = "<span class='fundPaymentSpan'>(" +
                investAmount.toLocaleString() + ")</span>";

    }
    // totals
    total += previous;
    totalInv += previousInv;
    totalCarr += previousCarr;
    totalInvest += previousInvest;

    row.insertCell(FundTotalsCarry.pastYears+3).innerHTML = total.toLocaleString();
    rowInv.insertCell(FundTotalsCarry.pastYears+3).innerHTML = totalInv.toLocaleString();
    rowCarr.insertCell(FundTotalsCarry.pastYears+3).innerHTML = totalCarr.toLocaleString();
    if (includePayments)
        nextRow.insertCell(FundTotalsCarry.pastYears+3).innerHTML = "<span class='fundPaymentSpan'>(" + totalInvest.toLocaleString() + ")</span>";

}

/*
 * Fund Cash Flow Future (FCFFuture) currently doing 10 years
 */
function insertFCFFuture(fund, row, body, index) {
    var totalReturn = 0;
    var haveCarry = "";


    /*
     * A fund is either in an investment fund or a carry only fund. Note must check the commit funds first for
     * the case where a fund is both (it will only be in FundTotalsInv)
     */
    if (fund.id in FundInvestments.commitMap)
        years = FundTotalsInv.funds[fund.id];
    else {
        years = FundTotalsCarry.funds[fund.id];
        haveCarry = "*";
    }

    // years = fundReturnsByYear.fundsInFuture[fund.id];
    // fundYears = fundReturnsByYear.fundYearsInFuture;

    row.insertCell(0).innerHTML = "<a href='/fundPage?id=" + fund.id + "'>" + fund.name + haveCarry + "</a>";

    if ("capitalAccount" in fund && fund.capitalAccount != 0) {
        totalReturn = Math.round(Number(fund.capitalAccount)/1000);
        row.insertCell(1).innerHTML = totalReturn.toLocaleString();
    } else
        row.insertCell(1).innerHTML = "";

    row.cells[1].addEventListener("click", function() {addCashflowUpdate(fund.id,"valuation update")});
    row.cells[1].addEventListener("mouseover", function() {row.cells[1].style.border = "2px solid blue";});
    row.cells[1].addEventListener("mouseout", function() {row.cells[1].style.border = "0px";row.cells[1].style.borderRight = "2px solid gray";});

    if ("predictedReturn" in fund && fund.predictedReturn != -1) {
        totalReturn = Math.round(Number(fund.predictedReturn)/1000);
        row.insertCell(2).innerHTML = totalReturn.toLocaleString();
    } else
        row.insertCell(2).innerHTML = "";

    row.cells[2].addEventListener("click", function() {addCashflowUpdate(fund.id,"exit prediction")});
    row.cells[2].addEventListener("mouseover", function() {row.cells[2].style.border = "2px solid blue";});
    row.cells[2].addEventListener("mouseout", function() {row.cells[2].style.border = "0px";row.cells[2].style.borderRight = "2px solid gray";});
    var yr, ret;

    for (var j=3; j < fund.model.yearCount+3; j++) {
        yr = CurrentYear + j - 3;

        if (yr == CurrentYear)
            yr += 100;

        if (years.hasOwnProperty(yr))
            ret = Math.round(years[yr].return_amount/1000);
        else {
            // console.debug("insertFCFFuture: no yr! fund.name: ", fund.name);
            ret = 0;
        }
        row.insertCell(j).innerHTML = ret.toLocaleString();

    }

    // console.debug("insertCFfuture - (j, ret): " + angelParens(j,ret));


}

/*
 * Fund Cash Flow Future totals (FCFPredictions) currently doing 10 years in the future incl this year
 */
function insertFCFFutureTotals() {
    var totalsTable = document.getElementById("fundPredictionsTotalsTable");
    var body = totalsTable.tBodies[0];
    var rowInv;
    var rowCarr;
    var row;

    var amount, investAmount, amountInv, total = 0, totalInvest = 0, totalCarr = 0, totalInv = 0;
    var capTotal = 0, capTotalInv = 0, capTotalCarr = 0,
        predictTotal = 0, predictTotalInv = 0, predictTotalCarr = 0, grandTotal = 0;
    // var includePayments = $('input[name="includePayments"]').is(':checked');
    var includePayments = false;    // for now we do not support payment predictions
    var yearsInv, yearsCarr;

    rowInv = body.insertRow(-1);
    cell = rowInv.insertCell(0);
    cell.innerHTML = "<hr>";
    cell.colSpan = 15;
    rowInv = body.insertRow(-1);

    rowCarr = body.insertRow(-1);
    row = body.insertRow(-1);

    cell = row.insertCell(0);
    cell.innerHTML = "<hr>";
    cell.colSpan = 15;
    row = body.insertRow(-1);

    row.style.color = "blue";
    rowInv.style.color = "blue";
    rowCarr.style.color = "blue";


    if (includePayments) {
        var nextRow = body.insertRow(-1);
        nextRow.insertCell(0).innerHTML = "";
    }
    // years = fundReturnsByYear.fundYearsInFuture;
    yearsCarr = FundTotalsCarry.years;
    yearsInv = FundTotalsInv.years;

    for (i=0; i<Funds.list.length; i++) {
        fund = Funds.list[i];
        if (!checkFundsFilter(fund))
            continue;
        if ("capitalAccount" in fund && Number(fund.capitalAccount) > 0) {
            capTotal += Math.round(Number(fund.capitalAccount)/1000);
            if (fund.id in FundInvestments.carryMap && !(fund.id in FundInvestments.commitMap)) {
                FundPerformanceTotals.carryOnlyCap += Math.round(Number(fund.capitalAccount));
                capTotalCarr += Math.round(Number(fund.capitalAccount)/1000);
            } else
                capTotalInv += Math.round(Number(fund.capitalAccount)/1000);
        }
        if ("predictedReturn" in fund && Number(fund.predictedReturn) > 0) {
            predictTotal += Math.round(Number(fund.predictedReturn)/1000);
            if (fund.id in FundInvestments.carryMap && !(fund.id in FundInvestments.commitMap)) {
                FundPerformanceTotals.carryOnlyPredRet += Math.round(Number(fund.predictedReturn));
                predictTotalCarr += Math.round(Number(fund.predictedReturn)/1000);
            } else
                predictTotalInv += Math.round(Number(fund.predictedReturn)/1000);
        }

        // console.debug("showFCFutureTotals: (capitalAcct,predictedReturn) - " + angelParens(capTotal,predictTotal));
        // console.debug("showFCFutureTotals: (carronlycap,carryonlypred) - " + angelParens(FundPerformanceTotals.carryOnlyCap,FundPerformanceTotals.carryOnlyPredRet));
    }


    rowInv.insertCell(0).innerHTML = "Totals Commit";
    rowInv.insertCell(1).innerHTML = capTotalInv.toLocaleString();
    rowInv.insertCell(2).innerHTML = predictTotalInv.toLocaleString();

    rowCarr.insertCell(0).innerHTML = "Totals Carry Only";
    rowCarr.insertCell(1).innerHTML = capTotalCarr.toLocaleString();
    rowCarr.insertCell(2).innerHTML = predictTotalCarr.toLocaleString();

    row.insertCell(0).innerHTML = "Totals";
    row.insertCell(1).innerHTML = capTotal.toLocaleString();
    row.insertCell(2).innerHTML = predictTotal.toLocaleString();


    for (j=3; j < 13; j++) {
        year = CurrentYear+j-3;
        amount = 0;
        investAmount = 0;


        if (year == CurrentYear)
            year += 100;            // hack to handle actuals and predictions in same (current) year


        if (yearsCarr.hasOwnProperty(year)) {
            amount = Math.round(yearsCarr[year].return_amount/1000);
            totalCarr += amount;

        }
        rowCarr.insertCell(j).innerHTML = amount.toLocaleString();


        amountInv = 0;
        if (yearsInv.hasOwnProperty(year)) {
            amountInv = Math.round(yearsInv[year].return_amount/1000);
            totalInv += amountInv;
            amount += amountInv;
            investAmount += Math.round(yearsInv[year].invest_amount/1000); // not currently in use
        }
        rowInv.insertCell(j).innerHTML = amountInv.toLocaleString();
        total += amount;
        totalInvest += investAmount;

        row.insertCell(j).innerHTML = amount.toLocaleString();
        if (includePayments)
            nextRow.insertCell(j).innerHTML = "<span class='fundPaymentSpan'>(" + investAmount.toLocaleString() + ")</span>";
    }


}

function addCashflowUpdate(id, type) {
    console.debug("addCashflowUpdate: id: " + id);


    var fundName = document.getElementById('cashflowUpdateFundName');
    fund = Funds.map[(id).toString()];

    if (type == "exit prediction") {
        $('#cashflowUpdateType').html("Predicted Value");
        if (fund.predictedReturn >= 0)
            $('input[name=cashflowValuation]').val(fund.predictedReturn);
        else
            $('input[name=cashflowValuation]').val("");
    } else if (type == "valuation update") {
        $('#cashflowUpdateType').html("Capital Account");
        if (fund.capitalAccount >= 0)
            $('input[name=cashflowValuation]').val(fund.capitalAccount);
        else
            $('input[name=cashflowValuation]').val("");
    }

    fundName.innerHTML = "<span style='color:blue'>" + fund.name + "</span>";


    $("#dialogCashflowUpdate").dialog({
        resizable: false,
        height: 450,
        width: 600,
        modal: true,
        buttons: {
            Proceed: function() {
                proceed(id, type);
                $(this).dialog("close");
            },
            Cancel: function() {
                $(this).dialog("close");
            }
        }
    });

    function proceed(id,type){

        var valuation = $('input[name=cashflowValuation]').val();
        var modelType = $('input[name=cashflowModel]:checked').val();
        var modelQS = "";

        if (modelType == "standard")
            modelQS = "standard";
        else {

            for (i=1; i <= 10; i++) {
                modelQS += i + "=" + $("#year"+i+"Cashflow").val() + "&";
            }
            modelQS = encodeURIComponent(modelQS);
        }
        console.debug("dialog Cashflow proceed: model qs: " + modelQS);

        $.get("fundPage?id="+id+ "&ajaxAddFundValuationUpdate&valuation="+valuation+"&fundUpdateType="+type+"&model="+modelQS,
            function(data,status) {
                clearCashflowTables();     // remove existing sort
                getAngelData();
                $(document).ajaxStop(function() {
                    showFundCashflows(InvestorID);
                    console.debug("fundPage valuation update return: " + JSON.stringify(data));
                    $.notify("Value updated", 'success');
                });
            });
    }

}       // addCashflowUpdate

/**************************************************
 * End of Funds
 */

/*
 ************************************************
 * Profiles, companies, people, notes, etc.
 */
function addOrUpdateCompanyNote() {
    var form = document.getElementById("addCompanyNote");

    var shrs = document.getElementById("companyFDshares");
    var val = document.getElementById("companyValuation");
    var pri = document.getElementById("companySharePrice");
    var epri = document.getElementById("companyExitSharePrice");
    var inv = document.getElementById("companyInvested");
    var spl = document.getElementById("companyStockSplitRatio");


    shrs.value =  $('#companyFDshares').val();      // the numbers are formatted in val() but not in the text value.
    val.value =  $('#companyValuation').val();
    pri.value =  $('#companySharePrice').val();
    epri.value =  $('#companyExitSharePrice').val();
    inv.value =  $('#companyInvested').val();
    spl.value =  $('#companyStockSplitRatio').val();

    console.debug("addOrUpdateCompanyNote: (shrs, val): price -- " + angelParens(shrs.value, val.value) + ": " + pri.value);

    if (addCompanyNoteValidation())
        form.submit();
    else
        angelJSerror("Please enter all required fields. Missing fields are red.")

}   // addOrUpdateCompanyNote

function companyNoteTypeSelector() {

    console.debug("companyNoteTypeSelector: type is: " + $("#companyNoteType").val());
    if ($("#companyNoteType").val() == "valuation update") {
        $("#companyValuationDiv").css("display", "inline");
        $("#companyExitPredictionDiv").css("display", "none");
        $("#companyStockSplitDiv").css("display", "none");
        $("#companyPublic").css("display", "none");
        $("#companyPublicText").html("Note: This information will never be public.");
        $("#companyPublic").prop("checked", 0);
    } else if ($("#companyNoteType").val() == "exit prediction") {
        $("#companyValuationDiv").css("display", "none");
        $("#companyStockSplitDiv").css("display", "none");
        $("#companyExitPredictionDiv").css("display", "inline");
        $("#companyPublic").css("display", "none");
        $("#companyPublicText").html("Note: This information will never be public.");
        $("#companyPublic").prop("checked", 0);
    } else if ($("#companyNoteType").val() == "stock split") {
        $("#companyValuationDiv").css("display", "none");
        $("#companyExitPredictionDiv").css("display", "none");
        $("#companyStockSplitDiv").css("display", "inline");
        $("#companyPublic").css("display", "none");
        $("#companyPublicText").html("");
        $("#companyPublic").prop("checked", 0);
    } else {
        $("#companyValuationDiv").css("display", "none");
        $("#companyStockSplitDiv").css("display", "none");
        $("#companyExitPredictionDiv").css("display", "none");
        $("#companyPublic").css("display", "inline");
        $("#companyPublicText").html("Public? If checked, this note will be visible to other Angelcalc users");
        $("#companyValuation").val(0);
        $("#companyFDshares").val(0);
    }

}

function addCompanyNoteValidation() {
    var allGood = true;


    if ($('#datepickerCompanyNote').val() == "") {
        $('#companyNoteDateHeader').css('color', 'red');
        allGood = false;
    } else
        $('#companyNoteDateHeader').css('color', 'black');

    if ($('#companyNote').val() == "") {
        $('#companyNoteNoteHeader').css('color', 'red');
        allGood = false;
    } else
        $('#companyNoteNoteHeader').css('color', 'black');


    return allGood;
}
function addOrUpdateCompany() {
    var form = document.getElementById("updateCompany");
    var invest = document.getElementById("proposedInvestment");

    /*
     * if we are adding / updating from dealflow, there is an extra proposedInvestment field we need to deal with
     */
    if ( $('#proposedInvestment').length != 0)          // if it exists - get the actual value
        invest.value =  $('#proposedInvestment').val();

    if (addCompanyValidation())
        form.submit();
    else
        angelJSerror("Please enter all required fields. Missing fields are red.")

}   // addOrUpdateCompany

function addCompanyValidation() {
    var allGood = true;


    if ($('#companyName').val() == "") {
        $('#companyNameLabel').css('color', 'red');
        allGood = false;
    } else
        $('#companyNameLabel').css('color', 'black');

    if ($('#companyShortdesc').val() == "") {
        $('#companyShortdescLabel').css('color', 'red');
        allGood = false;
    } else
        $('#companyShortdescLabel').css('color', 'black');


    return allGood;
}
/*
 * on a company profile page, ensure that they enter a new company name and don't select an existing company -
 * which is not allowed (we could allow it and then they'd be editing that company - but that's for later!).
 */
function companyMustBeNew() {

    var name = $('#companyName').val();
    if ($.isNumeric(name)) {
        $.notify("You cannot select an existing company - please type a new name.", "error");
        var action =  $('#companyEditAction').val();
        // console.debug("companyMustBeNew: action: " + action);
        var co = 0;
        if (action == "updateCompany") {
            var id = $('#companyID').val();
            var name = $('#savedCompanyName').val();
            console.debug("companyMustBeNew: (id,name): " + angelParens(id, name));
            var co = {id:id, text:name};
        }

        $("#companyName").select2("data", co);
    }


}

/*
 * on a fund profile page, ensure that they enter a new fund name and don't select an existing fund -
 * which is not allowed (we could allow it and then they'd be editing that fund - but that's for later!).
 */
function fundMustBeNew() {

    var name = $('#fundName').val();
    if ($.isNumeric(name)) {
        $.notify("You cannot select an existing fund - please type a new name.", "error");
        var action =  $('#fundEditAction').val();
        // console.debug("fundMustBeNew: action: " + action);
        var co = 0;
        if (action == "updateFund") {
            var id = $('#fundID').val();
            var name = $('#savedFundName').val();
            console.debug("fundMustBeNew: (id,name): " + angelParens(id, name));
            var co = {id:id, text:name};
        }

        $("#fundName").select2("data", co);
    }


}
/***********************************************
 * The input element is a select company ID.  Do an ajax request to get that company's data if it is not in
 * our list - which if this is a prospect it generally won't be - and load the data into the profile form.
 */

function loadCompanyProfile(element) {
    companyID = $(element).val();
    console.debug("loadCompanyProfile - companyID is: " + companyID);

    if ($.isNumeric(companyID))     // it's an existing company. grab its info.
        $.get("/companyPage?ajaxCompany&id=" + companyID, function(data,status) {

            var co = parseQueryStringToDictionary(data);
            if (co != null) {
                $('#companyShortdesc').val(co.shortdesc);
                $('#companyDescription').val(co.description);
                $('#companyURL').val(co.url);
                $('#companyOtherNames').val(co.other_names);

                /*
                 * Damn, need a sector map - since this co.sector is the value and not the id
                 */
               /* $("#e5").select2("val", null);
                $("#e5").select2("val", co.sector);*/

            }


        });

}

/*****************************************************************
 * on button push for adding or updating a person - validate fields and submit.
 *
 */
function addOrUpdatePerson() {
    var form = document.getElementById("updatePerson");

    if (addPersonValidation())
        form.submit();
    else
        angelJSerror("Please enter all required fields. Missing fields are red.")

}   // addOrUpdateCompany

function addPersonValidation() {
    var allGood = true;


    if ($('#userFirstName').val() == "") {
        $('#userFirstNameLabel').css('color', 'red');
        allGood = false;
    } else
        $('#userFirstNameLabel').css('color', 'black');

    if ($('#userLastName').val() == "") {
        $('#userLastNameLabel').css('color', 'red');
        allGood = false;
    } else
        $('#userLastNameLabel').css('color', 'black');

    if ($('#userEmail').val() == "") {
        $('#userEmailLabel').css('color', 'red');
        allGood = false;
    } else
        $('#userEmailLabel').css('color', 'black');



    return allGood;
}
/***********************************************************************************************************
 * Utility Functions - prob should move to another library
 */
function removeNewlinesAndSlashes(str) {
    if (str == null || str == "")
        return("");

    str = str.replace(/\\r\\n/g, '\n');      // remove c newlines
    str = str.replace(/\\/g, "");            // remove all backslashes - not sure this is really what I want
                                             // since this means you cannot have any!
    return(str);
}

function angelParens(left, right) {
    return("(" + left + "," + right + ")");
}

/*
 * called from html to close and keep closed divs usually with help text.
 */
function closeDiv(id) {
    $("#"+id).css("display","none");
    Cookies.set("angelcalc-"+id, "none");
}

/****************************************************
 *
 *  Graphics
 */

function pieGraphEquity(equity, div) {
    // Load the Visualization API and the corechart package.
    console.debug("calling piegraphequity: " + Math.round(equity.postFDShares - equity.totalShares-equity.convertShares-equity.founderTotalEquity));

    if (firstGraphicsCall) {
        firstGraphicsCall = false;
        google.charts.load('current', {'packages':['corechart']});
    }

    // Set a callback to run when the Google Visualization API is loaded.
    google.charts.setOnLoadCallback(drawChart);

    // Callback that creates and populates a data table,
    // instantiates the pie chart, passes in the data and
    // draws it.
    function drawChart() {

        // Create the data table.
        var data = new google.visualization.DataTable();
        data.addColumn('string', 'Shareholder');
        data.addColumn('number', 'Ownership');
        data.addRows([
            ['VC', Math.round(equity.totalShares)],
            ['Convert', Math.round(equity.convertShares)],
            ['Founder', Math.round(equity.founderTotalEquity)],
            ['Other Common', Math.round(equity.postFDShares - equity.totalShares-equity.convertShares-equity.founderTotalEquity-equity.postOptions)],
            ['Options', Math.round(equity.postOptions)]
        ]);

        // Set chart options
        var options = {'title':'',
            'width':350,
            'height':250
            //.,is3D: true

        };

        // Instantiate and draw our chart, passing in some options.
        var chart = new google.visualization.PieChart(document.getElementById(div));
        chart.draw(data, options);

    }
}
function pieGraphInvestStatus(div) {
    // Load the Visualization API and the corechart package.
    console.debug("piegraphinveststatus");

    try {
        if (firstGraphicsCall) {
            firstGraphicsCall = false;
            google.charts.load('current', {'packages':['corechart']});
        }

        // Set a callback to run when the Google Visualization API is loaded.
        google.charts.setOnLoadCallback(drawChart);
    } catch ( e ) {
        // not quite sure what I should do here. for now just return
        console.debug ("Google graphics call failed in pieGraphInvestStatus");
        return false;
    }

    // Callback that creates and populates a data table,
    // instantiates the pie chart, passes in the data and
    // draws it.
    function drawChart() {

        // Create the data table.
        var data = new pieGraphStatusData();

        // Set chart options
        var tit = "Invested in " + Companies.count  + " Companies";
        var options =
        {
            titleTextStyle: {color:'blue', fontSize:12},
            title:          tit,
            pieSliceText:   'value',
            legend: {position: 'right', maxLines: 2, textStyle: {color:"blue",fontSize: 12}},
            width:          200,
            height:         250,
            is3D:           true,
            slices:  {
                0: {color: 'blue'},         // active
                1: {color: 'green'},        // acqired
                2: {color: 'black'},        // ipo'd
                3: {color: 'red'}           // dead
            }

        };

        // Instantiate and draw our chart, passing in some options.
        var chart = new google.visualization.PieChart(document.getElementById(div));
        chart.draw(data, options);

    }
}

function pieGraphStatusData() {
    var data = new google.visualization.DataTable();

    var active = 0;          // active companies
    var dead = 0;
    var ipo = 0;            // dead: positive return
    var acquired = 0;       // dead: partial loss


    for (i in Companies.map) {
        co = Companies.map[i];
        if (co != null) {
            if (co.status == "active")
                active++;
            else if (co.status == "acquired")
                acquired++;
            else if (co.status == "ipo")
                ipo++;
            else if (co.status == "dead")
                dead++;
        }

    }

    data.addColumn('string', 'Status');
    data.addColumn('number', 'Count');
    data.addRows([
        ['Active', active],
        ['Acquired', acquired],
        ['IPO', ipo],
        ['Dead', dead]
    ]);

    return(data);
}

function comboGraphInvestResults(investSummary, div) {
    // Load the Visualization API and the corechart package.
    console.debug("calling graph");
    try {
        if (firstGraphicsCall) {
            firstGraphicsCall = false;
            google.charts.load('current', {'packages':['corechart']});
        }

        // Set a callback to run when the Google Visualization API is loaded.

            google.charts.setOnLoadCallback(drawChart);
    } catch ( e ) {
        // not quite sure what I should do here. for now just return
        console.debug ("Google graphics call failed in comboGraphInvestResults");
        return false;
    }

    // Callback that creates and populates a data table,
    // instantiates the pie chart, passes in the data and
    // draws it.
    function drawChart() {

        // Create the data table.
        var data = comboGraphData();
        var max = 5000000;
        // Set chart options
        var options = {
            title:'Performance Over Time',
            width:420,
            height:250,
            seriesType: 'bars',
            legend: {position: 'bottom', maxLines: 4},
            series: {
                0: {type: 'bar', color: 'red'},
                1: {type: 'bar', color: 'green'},
                2: {type: 'line', color: 'blue', visibleInLegend: true, targetAxisIndex:1}
                // 3: {type: 'line', color: 'green', visibleInLegend: false, targetAxisIndex:1}
            },
            vAxes: {0: {viewWindowMode:'explicit',
                viewWindow:{

                    min:0
                },
                gridlines: {color: 'transparent'}
            },
                1: {gridlines: {color: 'transparent'},
                    format:"#%"
                }
            }
        };

        // Instantiate and draw our chart, passing in some options.
        var chart = new google.visualization.ComboChart(document.getElementById(div));
        chart.draw(data, options);

    }
}

function comboGraphData() {
    var data = new google.visualization.DataTable();
    fundReturnsByYear.clear();

    for (i=0; i<Investments.list.length; i++) {
        fundReturnsByYear.add(Investments.list[i], false);
    }
    for (i=0; i<Payouts.list.length; i++) {
        fundReturnsByYear.add(Payouts.list[i], true);
    }
    // console.debug("combographdata: " + Object.keys(fundReturnsByYear.years));
    data.addColumn('string', 'Year');
    data.addColumn('number', 'Invested');
    data.addColumn('number', 'Returned');
    data.addColumn('number', 'ROI');
    //data.addColumn('number', 'Cumulative Ret')

    // for (year in  Object.keys(InvestmentsByYear.years)) {
    var cumulativeInv = 0;
    var cumulativeRet = 0;
    for (year in  fundReturnsByYear.years) {
        //console.debug("--- year:" + year);

        cumulativeInv += Number(fundReturnsByYear.years[year].invest_amount);
        cumulativeRet += Number(fundReturnsByYear.years[year].return_amount);

        if (cumulativeInv == 0)
            totalROI = 0;
        else
            totalROI = (cumulativeRet-cumulativeInv)/cumulativeInv;

        data.addRows([
            [year,
                fundReturnsByYear.years[year].invest_amount,
                fundReturnsByYear.years[year].return_amount,
                totalROI
            ]
        ]);

    }

    return(data);


}

function comboGraphFundResults(div) {
    // Load the Visualization API and the corechart package.
    console.debug("comboGraphFundResults. Div: " + div);

    if (firstGraphicsCall) {
        firstGraphicsCall = false;
        google.charts.load('current', {'packages':['corechart']});
    }

    // Set a callback to run when the Google Visualization API is loaded.
    google.charts.setOnLoadCallback(drawChart);

    // Callback that creates and populates a data table,
    // instantiates the pie chart, passes in the data and
    // draws it.
    function drawChart() {

        // Create the data table.
        var data = comboGraphFundData();
        var max = 5000000;
        // Set chart options
        var options = {
            title:'Fund Performance Over Time',
            width:800,
            height:250,
            seriesType: 'bars',
            legend: {position: 'bottom', maxLines: 4},
            series: {
                0: {type: 'bar', color: 'red'},
                1: {type: 'bar', color: 'green'},
                2: {type: 'line', color: 'blue', visibleInLegend: true, targetAxisIndex:1}
                // 3: {type: 'line', color: 'green', visibleInLegend: false, targetAxisIndex:1}
            },
            vAxes: {0: {viewWindowMode:'explicit',
                viewWindow:{

                    min:0
                },
                gridlines: {color: 'transparent'}
            },
                1: {gridlines: {color: 'transparent'},
                    format:"#%"
                }
            }
        };

        // Instantiate and draw our chart, passing in some options.
        var chart = new google.visualization.ComboChart(document.getElementById(div));
        chart.draw(data, options);

    }
}

function comboGraphFundData() {
    var data = new google.visualization.DataTable();
    fundReturnsByYear.clear();

    for (i=0; i<Investments.list.length; i++) {
        fundReturnsByYear.add(Investments.list[i], false);
    }
    for (i=0; i<Payouts.list.length; i++) {
        fundReturnsByYear.add(Payouts.list[i], true);
    }
    // console.debug("combographdata: " + Object.keys(fundReturnsByYear.years));
    data.addColumn('string', 'Year');
    data.addColumn('number', 'Invested');
    data.addColumn('number', 'Returned');
    data.addColumn('number', 'ROI');
    //data.addColumn('number', 'Cumulative Ret')

    // for (year in  Object.keys(InvestmentsByYear.years)) {
    var cumulativeInv = 0;
    var cumulativeRet = 0;
    for (year in  fundReturnsByYear.years) {
        //console.debug("--- year:" + year);

        cumulativeInv += Number(fundReturnsByYear.years[year].invest_amount);
        cumulativeRet += Number(fundReturnsByYear.years[year].return_amount);

        if (cumulativeInv == 0)
            totalROI = 0;
        else
            totalROI = (cumulativeRet-cumulativeInv)/cumulativeInv;

        data.addRows([
            [year,
                fundReturnsByYear.years[year].invest_amount,
                fundReturnsByYear.years[year].return_amount,
                totalROI
            ]
        ]);

    }

    return(data);


}

function graphCompanyValueOverTime(div) {
    // Load the Visualization API and the corechart package.
    console.debug("calling graphCompanyValueOverTime");
    try {
        if (firstGraphicsCall) {
            firstGraphicsCall = false;
            google.charts.load('current', {'packages':['corechart']});
        }

        // Set a callback to run when the Google Visualization API is loaded.

        google.charts.setOnLoadCallback(drawChart);
    } catch ( e ) {
        // not quite sure what I should do here. for now just return
        console.debug ("Google graphics call failed in comboGraphInvestResults");
        return false;
    }

    // Callback that creates and populates a data table,
    // instantiates the pie chart, passes in the data and
    // draws it.
    function drawChart() {

        // Create the data table.
        var data = companyValueData();
        var max = 5000000;
        // Set chart options
        var options = {
            title:'Company Value Over Time (log scale)',
            titlePosition: "in",
            curveType: 'function',
            interpolateNulls: true,
            width:800,
            height:600,
            vAxis:{textPosition: 'out',
                textStyle: {color:'green'},
                logScale: true
            },
            legend: {position: 'top', maxLines: 8, textStyle: {color:"blue",fontSize: 10}}
        };

        // Instantiate and draw our chart, passing in some options.
        var chart = new google.visualization.LineChart(document.getElementById(div));
        chart.draw(data, options);

    }
}

function companyValueData() {

    var data = new google.visualization.DataTable();



    // console.debug("companyValueData: " + Object.keys(fundReturnsByYear.years));

    data.addColumn('string', 'Year');
    for (i=0; i<Companies.list.length; i++) {
        co = Companies.list[i];
        if ("valUpdates" in co) {
            data.addColumn('number', co.name);
        }
    }

    var years = [];
    for (i = 10; i >= 0; i--) {
        years.push(CurrentYear - i);
    }

    /*
     * first add the original investment to valUpdates
     */
    for (j=0; j<Companies.list.length; j++) {
        co = Companies.list[j];
        if (! ("valUpdates" in co) )    // this company has no valuation updates, so skip it
            continue;


        var inv = Companies.firstInvestment(co.id);
        if (inv != null) {
            if (!("cap" in inv))
                inv.cap = null;
            if (!("prevaluation" in inv))
                inv.prevaluation = null;

            // console.debug("first investment for co: (date, cap) premoney " + co.name + ": " + angelParens(inv.inv_date, inv.cap) + " " + inv.prevaluation);



            var dt = new Date(inv.inv_date);
            var yr = dt.getUTCFullYear();
            if (inv.cap != 0 && inv.cap != null)
                co.valUpdates[yr.toString()] = inv.cap;
            else if (inv.preMoneyVal != 0 && inv.prevaluation != null)
                co.valUpdates[yr.toString()] = inv.prevaluation;
        } else
            console.debug("first investment returns null");

    }

    for (i = 0; i < years.length; i++) {
        var year = years[i];
        var row = [];
        row.push(year.toString());
        for (j=0; j<Companies.list.length; j++) {
            co = Companies.list[j];
            if (! ("valUpdates" in co) )    // this company has no valuation updates, so skip it
                continue;


            if ("valUpdates" in co) {
                if (co.name == "Stripe") {
                    console.debug("stripe: " + JSON.stringify(co.valUpdates));
                }
                if (year in co.valUpdates)
                    row.push((Number(co.valUpdates[year])));
                else
                     row.push(null);

            }
        }
        // console.debug("row: " + JSON.stringify(row));

        data.addRows([row])
    }
    //console.debug("ok!: " + JSON.stringify(data));
    return(data);


}

function hideGraphics() {
    //console.debug("hiding graphics");

    var graph1 = document.getElementById("summaryGraph1");
    var graph2 = document.getElementById("summaryGraph2");

    graph1.style.display = "none";
    graph2.style.display = "none";
}

function showGraphics() {

    //console.debug("showing graphics");

    var graph1 = document.getElementById("summaryGraph1");
    var graph2 = document.getElementById("summaryGraph2");

    graph1.style.display = "inline";
    graph2.style.display = "inline";

    $("#transparentHelpDiv").css("display","none"); /* make sure the initial help is gone */
}

function IRR(values, guess) {
    // Credits: algorithm inspired by Apache OpenOffice

    // Calculates the resulting amount
    var irrResult = function(values, dates, rate) {
        var r = rate + 1;
        var result = values[0];
        for (var i = 1; i < values.length; i++) {
            result += values[i] / Math.pow(r, (dates[i] - dates[0]) / 365);
        }
        return result;
    }

    // Calculates the first derivation
    var irrResultDeriv = function(values, dates, rate) {
        var r = rate + 1;
        var result = 0;
        for (var i = 1; i < values.length; i++) {
            var frac = (dates[i] - dates[0]) / 365;
            result -= frac * values[i] / Math.pow(r, frac + 1);
        }
        return result;
    }

    // Initialize dates and check that values contains at least one positive value and one negative value
    var dates = [];
    var positive = false;
    var negative = false;
    for (var i = 0; i < values.length; i++) {
        dates[i] = (i === 0) ? 0 : dates[i - 1] + 365;
        if (values[i] > 0) positive = true;
        if (values[i] < 0) negative = true;
    }
    // (Geoff) Return error if first value is not negative (an investment)
    if (values[0] >= 0)
        return 'error';

    // Return error if values does not contain at least one positive value and one negative value
    if (!positive || !negative) return 'error';

    // Initialize guess and resultRate
    var guess = (typeof guess === 'undefined') ? 0.1 : guess;
    var resultRate = guess;

    // Set maximum epsilon for end of iteration
    var epsMax = 1e-10;

    // Set maximum number of iterations
    var iterMax = 50;

    // Implement Newton's method
    var newRate, epsRate, resultValue;
    var iteration = 0;
    var contLoop = true;
    do {
        resultValue = irrResult(values, dates, resultRate);
        newRate = resultRate - resultValue / irrResultDeriv(values, dates, resultRate);
        epsRate = Math.abs(newRate - resultRate);
        resultRate = newRate;
        contLoop = (epsRate > epsMax) && (Math.abs(resultValue) > epsMax);
    } while(contLoop && (++iteration < iterMax));

    if(contLoop) return 'error';

    // Return internal rate of return
    return resultRate;
}

/*****************************************************
 * alert user that a JS error has taken place.
 * @param err
 */

function angelJSerror(err) {

    $.notify(err, "error");

}
/**********************************************************
 * Deprecated stuff
 */


function OLDcalcOptions(disc) {
    // input to calculations

    var A   = Number(document.getElementById('convTotal').value);       // total amount converts invested
    var D    = Number(document.getElementById('discount').value);
    var C         = Number(document.getElementById('cap').value);
    var I    = Number(document.getElementById('investAmount').value);    // VC investment
    var V         = Number(document.getElementById('valuation').value);
    var PP   = Number(document.getElementById('opoolPost').value);        // option percentage post financing
    var S     = Number(document.getElementById('sharesOut').value);   // total issued shares (does not include options)

    var postOptionCount;


    var specialOptions = document.getElementById('specialOptions');

    PP = PP/100;
    D = (1-D/100);
    if (disc)
        postOptionCount = (W(PP,I,V)*S*A + W(PP, I, V) * S * (D * V - A)) / ((X(I,V,PP) * ((D * V) - A)) - W(PP,I,V)*A);
    else
        postOptionCount = (Z(I,V,PP,S,C) + ZZ(I, V, PP, S, A)) / (X(I,V,PP) * C - Y(I,V,PP,A));

    return(postOptionCount);
    // specialOptions.innerHTML = Number(O.toFixed(0)).toLocaleString() + " (" + String(PP*100) + "%)";


}

function X(I, V, PP) {
    return(I * (V - PP * (I + V)));
}

function Y(I, V, PP, A) {
    return(W(PP,I,V) * A);
}

function Z(I, V, PP, S, C) {
    return(W(PP,I,V)  * S * C);
}

function ZZ(I, V, PP, S, A) {
    return(W(PP,I,V)  * S * A);
}

function W(PP, I, V) {
    return((PP * I * V )+ (PP * I * I));
}

function ZZZ(I, V, PP, S, A) {
    return(W(PP,I,V)  * S);
}
