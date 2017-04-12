

function ajax_load(url, elemToReplace) 
{
    var xhttp = new XMLHttpRequest();
    xhttp.onreadystatechange = function() {
        if (xhttp.readyState == 4 && xhttp.status == 200)
        {

            if(elemToReplace != null)
            {
                domElemToReplace = document.getElementById(elemToReplace);

                if(domElemToReplace)
                        domElemToReplace.innerHTML = xhttp.responseText;

            }
        }
    };

    xhttp.open("GET", url, true);
    xhttp.send();
}

function ajax_post(url, form_data, elemToReplace, on_post_cb)
{
    var xhttp = new XMLHttpRequest();

    xhttp.onreadystatechange = function() {
        if (xhttp.readyState == 4 && xhttp.status == 200) {

            if(on_post_cb != null)
            {
                on_post_cb(xhttp.responseText);
            }
            else if(elemToReplace != null)
            {
                var domElemToReplace = document.getElementById(elemToReplace);

                if(domElemToReplace)
                    domElemToReplace.innerHTML = xhttp.responseText;
            }
        }
    };

    xhttp.open("POST", url, true);

    xhttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");

    xhttp.send(form_data);
}

function on_frame_slider(iFrame)
{
    ajax_load('/set_log_frame?frame=' + iFrame);
}
