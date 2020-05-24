/**
 * Created by geoff on 6/22/16.
 */

$(document).ready(function () {

    //Prepare jTable
    $('#tableViewerContent').jtable({
        title: 'Angelcalc Companies',
        paging: true,
        sorting: true,
        defaultSorting: 'name ASC',
        pageSize: 15,
        openChildAsAccordion: true,
        actions: {

                listAction:   '/tables/companyActions.php?action=list',
                createAction: '/tablescompanyActions.php?action=create',
                updateAction: '/tables/companyActions.php?action=update',
                deleteAction: '/tables/companyActions.php?action=delete'

    },
    toolbar: {
        hoverAnimation: true, //Enable/disable small animation on mouse hover to a toolbar item.
        hoverAnimationDuration: 60, //Duration of the hover animation.
        hoverAnimationEasing: undefined, //Easing of the hover animation. Uses jQuery's default animation ('swing') if set to undefined.
        items: []
        },
    fields: {          /* note that these must be identical to the values returned by mysql! */
    id: {
                title: 'ID',
                key: true,
                create: false,
                edit: false,
                list: true
            },
        Founders: {
            title: '',
            width: '5%',
            sorting: false,
            edit: false,
            create: false,
            display: function (companyData) {
                //Create an image that will be used to open child table
                var $img = $('<img src="/img/profile.png" heigh=20 width=20 title="Company Founders" />');
                //Open child table when user clicks the image
                $img.click(function () {
                    $('#CompanyTableContainer').jtable('openChildTable',
                        $img.closest('tr'),
                        {
                            title: companyData.record.name + ' - Founders',
                            actions: {
                                listAction: 'utility/FounderList.php?action=list&FounderIDs=' + companyData.record.founder_ids,
                                deleteAction: 'utility/FounderList.php?action=delete',
                                updateAction: 'utility/FounderList.php?action=update',
                                createAction: 'utility/FounderList.php?action=create'
                            },
                            fields: {
                                company_id: {
                                    type: 'hidden',
                                    defaultValue: companyData.record.id
                                },
                                firstname: {
                                    title: 'First Name',
                                    width: '20%'
                                },
                                lastname: {
                                    title: 'Last Name',
                                    width: '20%'
                                },
                                email: {
                                    title: 'Email',
                                    width: '20%'
                                },
                                cell: {
                                    title: 'cell',
                                    width: '20%'
                                },
                                id: {
                                    visibility: 'hidden',
                                    title: "id",
                                    key: true,
                                    create: false
                                },
                                last_update: {
                                    title: 'Record date',
                                    visibility: 'hidden',
                                    width: '20%',
                                    type: 'date',
                                    displayFormat: 'yy-mm-dd',
                                    create: false,
                                    edit: false
                                }
                            }
                        }, function (data) { //opened handler
                            data.childTable.jtable('load');
                        });
                });
                //Return image to show on the person row
                return $img;
            }
        },

        name: {
            title: 'Name',
            width: '10%',
            columnResizeable : true,
            visibility: 'fixed',
            display: function (data) {
                link = "<a href=/companypage.php?id=" + data.record.id + ">" + data.record.name + "</a>";
                return link;
                //return 'test';
            }
        },
        other_names: {
            title: 'Other Names',
            width: '10%',
            columnResizeable : true
        },
        yr_founded: {
            title: 'Founded',
            width: '10%',
            columnResizeable : true
        },
        yr_closed: {
            title: 'Closed',
            visibility: 'hidden',
            width: '10%',
            columnResizeable : true
        },
        kind: {
            title: 'Kind',
            width: '10%',
            options: {'startup': 'startup', 'vc' : 'vc','corporation': 'corporation'},
            defaultValue: '',
            columnResizeable : true
        },
        status: {
            title: 'Status',
            width: '10%',
            options: {'unknown': 'unknown', 'active' : 'active', 'dead' : 'dead', 'acquired' : 'acquired', 'ipo' : 'ipo'},
            defaultValue: '',
            columnResizeable : true
        },
        sector: {
            title: 'Sector',
            width: '10%',
            columnResizeable : true
        },
        YCbatch: {
            title: 'Batch',
            width: '10%',
            columnResizeable : true
        },
        creatorID: {
            title: 'Creator',
            width: '10%',
            columnResizeable : true
        },
        FDshares: {
            title: 'FD Shares',
            width: '10%',
            columnResizeable : true
        },
        valuation: {
            title: 'Val',
            width: '10%',
            columnResizeable : true
        },
        description: {
            title: 'Description',
            visibility: 'hidden',
            columnResizeable : true,
            width: '30%'
        },
        shortdesc: {
            title: 'Desc',
            visibility: 'hidden',
            columnResizeable : true,
            width: '30%'
        },
        location: {
            title: 'Location',
            width: '15%'
        },
        email: {
            title: 'email',
            width: '10%'
        },
        logo: {
            title: 'logo',
            width: '10%'
        },
        url: {
            title: 'URL',
            width: '10%'
        },

        founder_ids: {
            title: 'Founder IDs',
            visibility: 'hidden',
            width: '10%'
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
            kind: $('#kind').val(),
            status: $('#status').val()
        });
    });

    //Load all records when page is first shown
    $('#LoadRecordsButton').click();


});
