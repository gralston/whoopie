/**
 * Created by geoff on 6/22/16.
 */

$(document).ready(function () {


    $.get("/investments?loadModelandInvestCounts", function(data,status) {
        var allData = JSON.parse(data);
        var models = allData.models;
        var modelMap = {};
        var mod;
        var invests = allData.investments;
        var investMap = {};
        var inv;
        console.debug("angelTabPerson: " + JSON.stringify(allData));

        for (var i=0; i < models.length; i++) {
            mod = models[i];
            modelMap[mod[0]] = mod[1];
        }
        for (var i=0; i < invests.length; i++) {
            inv = invests[i];
            investMap[inv[0]] = inv[1];
        }



    //Prepare jTable
    $('#tableViewerContent').jtable({
        title: 'Angelcalc People',
        paging: true,
        sorting: true,
        defaultSorting: 'lastname ASC',
        pageSize: 15,
        openChildAsAccordion: true,
        actions: {

                listAction:   '/tables/personActions.php?action=list',
                createAction: '/tables/personActions.php?action=create',
                updateAction: '/tables/personActions.php?action=update',
                deleteAction: '/tables/personActions.php?action=delete'

    },
    toolbar: {
        hoverAnimation: true, //Enable/disable small animation on mouse hover to a toolbar item.
        hoverAnimationDuration: 60, //Duration of the hover animation.
        hoverAnimationEasing: undefined, //Easing of the hover animation. Uses jQuery's default animation ('swing') if set to undefined.
        items: []
        },
    fields: {          /* note that these must be identical to the values returned by mysql! */
    id: {
        width: "5%",
        title: 'ID',
        key: true,
        create: false,
        edit: false,
        list: true
            },
    firstname: {
        title: 'firstname',
        width: '10%',
        columnResizeable : true,
        visibility: 'fixed',
        display: function (data) {
        link = "<a href=/userProfile.php?id=" + data.record.id + ">" + data.record.firstname + "</a>";
        return link;
        //return 'test';
        }
    },
    lastname: {
        title: 'lastname',
        width: '10%',
        columnResizeable : true,
        visibility: 'fixed',
        display: function (data) {
        link = "<a href=/userProfile.php?id=" + data.record.id + ">" + data.record.lastname + "</a>";
        return link;
        //return 'test';
        }
    },
    gender: {
        title: 'gender',
        visibility: 'hidden',
        options: {'m' : 'male', 'f' : 'female', 'o': 'other'},
    defaultValue: 'male',
    width: '10%',
    columnResizeable : true
    },
    email: {
        title: 'email',
        width: '10%',
        columnResizeable : true
        },
    password: {
        title: 'password',
        width: '10%',
        visibility: 'hidden',
        columnResizeable : true
        },
    regdate: {
        title: 'Reg Date',
        width: '10%',
        columnResizeable : true
        },
    cell: {
        title: 'Cell',
        visibility: 'hidden',
        columnResizeable : true,
        width: '10%'
        },
    photo: {
        title: 'Photo',
        visibility: 'hidden',
        columnResizeable : true,
        width: '10%'
        },
    dob: {
        title: 'DOB',
        visibility: 'hidden',
        width: '10%'
        },
    models: {
        title: 'models',
        width: '5%',
        columnResizeable : true,
        visibility: 'fixed',
        display: function (data) {
            return modelMap[data.record.id];
        }
    },
    invests: {
        title: 'invests',
        width: '5%',
        columnResizeable : true,
        visibility: 'fixed',
        display: function (data) {
            return investMap[data.record.id];
        }
    },
    kind: {
        title: 'kind',
        visibility: 'hidden',
        options: {'ik12founder' : 'ik12founder',
                  'founder' : 'founder', 'angel': 'angel','vcperson':'vcperson','educator':'educator',
                   'ik12person':'ik12person','angelcalcfounder' : 'angelcalcfounder', 'user':'user'},
    defaultValue: 'founder',
    width: '10%'
    },
    is_an_angel: {
        title: 'Angel?',
        visibility: 'hidden',
        type: 'checkbox',
        values: {0: "", 1: "Yes"},
    defaultValue: 0,
    width: '10%'
    },
    former_ik12founder: {
        title: 'Was IK12?',
        visibility: 'hidden',
        type: 'checkbox',
        values: {0: "", 1: "Yes"},
    defaultValue: 0,
    width: '10%'
    },
    description: {
        title: 'desc',
        width: '20%',
        visibility: 'hidden',
        columnResizeable : true
        },

    company_id: {
        title: 'Company ID',  // fix this to display the actual company with an appropriate link
        visibility: 'hidden',
        width: '10%'
        },

    access: {
        title: 'Access',
        visibility: 'hidden',
        width: '10%'
        },
    hash: {
        title: 'Hash',
        visibility: 'hidden',
        width: '10%'
        },
    state: {
        title: 'State',
        visibility: 'hidden',
        options: {'static' : 'static', 'waitverify' : 'waitverify', 'verified': 'verified','partner': 'partner'},
        defaultValue: '',
        width: '7%'
    },
    last_update: {
        title: 'Updated',
        // width: '200px',
        type: 'date',
        visibility: 'hidden',
        create: false,
        edit: false
        }
    }
    });

    /*
    * Must add filters here as well.  Then re-load records when user click 'load records' button.
    */


    $('#LoadRecordsButton').click(function (e) {
        e.preventDefault();
        $('#tableViewerContent').jtable('load', {
            name: $('#name').val(),
            email: $('#email').val(),
            id: $('#id').val(),
            companyName: $('#companyName').val(),
            gender: $('#gender').val(),
            kind: $('#kind').val(),
            status: $('#status').val()
        });
    });

    //Load all records when page is first shown
    $('#LoadRecordsButton').click();

    }); // get

});
