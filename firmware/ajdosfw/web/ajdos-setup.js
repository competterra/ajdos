var hTimeOut   = false;
var hModalTime = false;
var readModal  = false;
var saveModal  = false;

function loadPage( page ) 
{
    if ( hModalTime )
    {
      clearTimeout( hModalTime );
      saveModal.hide();
    }
    hModalTime = false;
    if ( hTimeOut )
      clearTimeout( hTimeOut );
    hTimeOut = false;

    if ( $("#pg_"+page).length )
    {   
        $("#blank").html( $("#pg_"+page).html() );
        $("#breadcrumb").text( $("#pg_"+page).attr("data-title"));
        loadData();
    }
    $('.navbar-collapse').collapse('hide');
    return;  
};

function loadFinishPage( page ) 
{
    if ( hModalTime )
    {
      clearTimeout( hModalTime );
      saveModal.hide();
    }
    hModalTime = false;
    if ( hTimeOut )
      clearTimeout( hTimeOut );
    hTimeOut = false;

    if ( $("#pg_"+page).length )
    {   
        $("#blank").html( $("#pg_"+page).html() );
        $("#breadcrumb").text( $("#pg_"+page).attr("data-title"));
    }
    $('.navbar-collapse').collapse('hide');
    return;    
}


function loadData()
{
    
    if ( false === readModal )
        readModal = new bootstrap.Modal(document.getElementById('modal-reading'), { keyboard: false  });     
    if ( false === saveModal )
        saveModal = new bootstrap.Modal(document.getElementById('modal-saved'),   { keyboard: false  }); 

    saveModal.hide();
    readModal.show();
    
    var x = new XMLHttpRequest();
    x.open("GET", "/properties", true);
    x.onload = function ()
    {        
        let props = JSON.parse( x.responseText );
        fillData( props );                
        readModal.hide();
    }
    x.onerror = function()
    {
        readModal.hide();
        alert("Read error ("+x.status+")");
    }
    x.send(null);
    

    /*
    var fakeData = '{"main":{"name":"bp-test","lat":"47.43915","long":"19.21384","alt":"183","height":"3","password":"12345678","owner":"fincygo","station":"home"},';
    fakeData += '"measuring":{"cycle":"60", "lag":"0" },';
    fakeData += '"wifi":{"ssid":"WIFIName","pwd":"WIFIPass", "tested":false },';
    fakeData += '"send":{"ip":"185.68.73.135","port":"1883","user":"admin","pwd":"admin", "tested":false },';
    fakeData += '"test":{"done":"!", "wifi":"OK", "mqtt":"!" }}';
    let props = JSON.parse( fakeData );
    fillData( props );                
    readModal.hide();
    */
    
}

function fillData( props )
{
    for (let group in props )
    {
        if ( props.hasOwnProperty( group) )
        {                        
            for ( let field in props[group] )
            {
                if ( props[group].hasOwnProperty( field ) )
                {
                    let fieldId = "#set-"+group+"-"+field;
                    if ( $(fieldId).length )
                    {
                      if ( Number.isFinite( props[group][field] ) && props[group][field] == 0 )
                        $(fieldId).val("");
                      else  
                        $(fieldId).val(props[group][field]);
                    }
                }
            }
            
        }
    }   
}

function getLocation()
{
  if (navigator.geolocation)
  {
    navigator.geolocation.getCurrentPosition(showPosition, errorPosition );
  }
  else
    $("#msg-geo").text("Geolocation is not supported by this browser.");
}

function showPosition( pos )
{
    $('#set-main-lat').val(pos.coords.latitude);
    $('#set-main-long').val(pos.coords.longitude);
}

function errorPosition(e)
{
    switch (e.code)
    {
    case e.PERMISSION_DENIED:
        $("#msg-geo").text("User denied the request for Geolocation.");
        break;
    case e.POSITION_UNAVAILABLE:
        $("#msg-geo").text("Location information is unavailable.");
        break;
    case e.TIMEOUT:
        $("#msg-geo").text("The request to get user location timed out.");
        break;
    case e.UNKNOWN_ERROR:
        $("#msg-geo").text("An unknown position.");
        break;        
    }
}

function listWifiNetworks()
{   
    $("#wireles-list").empty();
    var x = new XMLHttpRequest();
    x.open("GET", "/wifilist", true);
    x.onload = function ()
    {
        let list = JSON.parse( x.responseText );
        for ( item in list.ssid )
            addWifiToList( item, list.ssid[item]);
    }
    x.send(null);
}

function addWifiToList( n, name )
{
    let newItem = $("#template-wifi-item").html().trim();
    newItem = newItem.replace("><",">"+name+"<").replace(/wi-/g,"wi-"+n);
    $("#wireles-list").append( newItem );
}

function selectWifi( index )
{
    $("#set-wifi-ssid").val( $( "#"+index ).text() );
}

function showAdminPasswords()
{
  let newType = ( $('#chkSap').is(':checked') ) ? "text":"password";
  $("#set-admin-pwdold").attr("type", newType );
  $("#set-admin-pwd1").attr("type", newType );
  $("#set-admin-pwd2").attr("type", newType );
}

function onload()
{     
    loadPage('home');
    loadData();
    if ($('#set-main-lat').val() == "" || $('#set-main-long').val() == "")
    {
        getLocation();
    }
};

function reboot()
{
    loadFinishPage("rebooting");
    var x = new XMLHttpRequest();        
    x.open("GET", "/reboot", true);
    x.send(null);
}

function test()
{
    loadFinishPage("testing");
    var x = new XMLHttpRequest();        
    x.open("GET", "/test", true);
    x.send(null);
    hTimeOut = setTimeout( function() { location.reload(); clearTimeout(hTimeOut); hTimeOut=false; }, 180*1000);
}

function resetsensor()
{
    loadFinishPage("reseting");
    var x = new XMLHttpRequest();        
    x.open("GET", "/resetsensor", true);
    x.send(null);
}


function notEmpty( inp )
{
  if ( $('#'+inp).val() )
  {
    $('#'+inp).css('border-width','1px');
    $('#'+inp).css('border-color','#ced4da');
    return true;
  }
  $('#'+inp).css('border-width','2px');
  $('#'+inp).css('border-color','red');
  return false;
}

function notNaN( inp )
{      
  if ( !notEmpty( inp ) )
    return false;
  if ( isNaN( $('#'+inp).val() ) ) 
  {
    $('#'+inp).css('border-width','2px');
    $('#'+inp).css('border-color','red');
    return false;
  }
  $('#'+inp).css('border-width','1px');
  $('#'+inp).css('border-color','#ced4da');
  return true;
}

function rangeTest( inp )
{
  let min = parseInt($('#'+inp).attr('min'));
  let max = parseInt($('#'+inp).attr('max'));
  let val = parseInt($('#'+inp).val());
  return ( (min <= val) && (val <= max) );
}

function ValidateIPaddress(ipaddress) 
{
  var ipformat = /^(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$/;
  if(ipaddress && ipaddress.match(ipformat))
  {
    $('#set-send-ip').css('border-width:1px; border-color:#ced4da;')
    return true;
  }
  $('#set-send-ip').css('border-width','2px'); 
  $('#set-send-ip').css('border-color','red');  
  return false;
}

function saveData( url, params, name )
{
  var x = new XMLHttpRequest();
  console.log(url+"?"+encodeURI(params));
  x.open("GET", url+"?"+encodeURI(params), true);
  x.onload = function ()
  {
    let result = JSON.parse( x.responseText );
    if ( result.code == 200 )
    {
      saveModal.show();
      hModalTime = setTimeout( function() { saveModal.hide(); clearTimeout(hModalTime); hModalTime = false; }, 1000);
    }
    else    
      console.log(name+" result:"+result.code );        
  }
  x.onerror = function()
  {
      alert(name + " not saved ("+x.status+")");    
  }
  x.send(null);
}

function saveHome()
{
  if ( notEmpty('set-main-name') && notEmpty('set-main-password') && notEmpty('set-main-owner') && notEmpty('set-main-station') && notNaN('set-main-lat') && notNaN('set-main-long') && notNaN('set-main-alt')) 
  {
    var param = "name="+$("#set-main-name").val()+"&";
    param += "password="+$("#set-main-password").val()+"&";
    param += "owner="+$("#set-main-owner").val()+"&";
    param += "station="+$("#set-main-station").val()+"&";
    param += "lat="+$("#set-main-lat").val().toString()+"&";
    param += "long="+$("#set-main-long").val().toString()+"&";
    param += "alt="+$("#set-main-alt").val().toString()+"&";
    param += "height="+$("#set-main-height").val().toString();
    saveData("/set-sensor", param, "Home");
  }
}

function saveMeasuring()
{  
  if ( rangeTest("set-measuring-cycle") && rangeTest("set-measuring-lag") )
  {    
    var param = "cycle='"+$("#set-measuring-cycle").val().toString()+"'&";
    param += "lag='"+$("#set-measuring-lag").val().toString()+"'";
    saveData("/set-measuring", param, "Measuring");
  }
}

function saveWifi()
{
  if ( notEmpty('set-wifi-ssid') && notEmpty('set-wifi-pwd') )
  {
    var param = "ssid="+$("#set-wifi-ssid").val()+"&";
    param += "pwd="+$("#set-wifi-pwd").val();
    saveData("/set-wifi", param, "WiFi");
  }
}

function saveSend()
{
  if ( notEmpty('set-send-ip') && notEmpty('set-send-port') && notEmpty( 'set-send-user') && notEmpty( 'set-send-pwd') && ValidateIPaddress($('#set-send-ip').val() ) )
  {
    var param = "ip="+$("#set-send-ip").val()+"&";
    param += "port="+$("#set-send-port").val().toString()+"&";
    param += "user="+$("#set-send-user").val()+"&";
    param += "pwd="+$("#set-send-pwd").val();
    saveData("/set-mqtt", param, "Broker");
  }
}

function saveAdmin()
{
  if ( notEmpty('set-admin-pwdold') && notEmpty('set-admin-pwd1') && notEmpty('set-admin-pwd2')  )
  {
    if ( $("#set-admin-pwd1").val() === $("#set-admin-pwd2").val() )
    {
      var param = "pwdold="+$("#set-admin-pwdold").val()+"&";
      param += "pwdnew="+$("#set-admin-pwd1").val();
      saveData("/set-admin", param, "Admin");      
    }
    else
      alert("Two passwords are not equal.")
  }
}

function showInfo( id )
{
  $("#"+id).show();
}
