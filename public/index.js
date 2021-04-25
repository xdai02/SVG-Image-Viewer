$(document).ready(function() {
    loadAllFiles(true);
    uploadFile();
    showSVGInfo();
});

// validate numeric value
function isNumber(val) {
    var regPos = /^\d+(\.\d+)?$/;
    var regNeg = /^(-(([0-9]+\.[0-9]*[1-9][0-9]*)|([0-9]*[1-9][0-9]*\.[0-9]+)|([0-9]*[1-9][0-9]*)))$/;
    if(regPos.test(val) || regNeg.test(val)) {
        return true;
    } else {
        return false;
    }
}

// load all files from the server
function loadAllFiles(flag) {
    $("#log-body").empty();

    // initialize and get all files on the server when the web page refreshes
    $.ajax({
        type: 'get', 
        dataType: 'json', 
        url: '/getFiles', 
        data: {

        },

        success: function(data) {
            var folder = '../uploads/';
            if(data.fileName.length == 0) {
                $("#log-body").append(
                    "<tr> <td colspan='7'>No Files</td></tr>"
                )
                $("#log-body").css("text-align", "center");
                $("#log-body").css("text-font", "18px");
                $("#log-body").css("height", "50px");
                $("#log-body").css("line-height", "50px");
            }
            if(data.fileName.length >= 5) {
                $(".file-log").css("height", "200%");
            }
            if(flag == true) {
                $("#file-drop-list").empty();
                $("#file-drop-list").append("<option>-- Select --</option>");
            }
            
            var cnt = 0;
            for(var file in data.fileName) {
                // add rows in file log panel
                $("#log-body").append(
                    "<tr>" + 
                    "<td> <a href='" + folder + data.fileName[file] + 
                    "' download='" + data.fileName[file] + "'>" + 
                    "<img id='thumbnail' src='" + folder + data.fileName[file] + "?t=" + Math.random() + "'</a>" +
                    "</td>" + 
                    "<td> <a href='" + folder + data.fileName[file] + 
                    "' download='" + data.fileName[file] + "'>" + data.fileName[file] + "</a>" + 
                    "</td>" + 
                    "<td>" + data.fileSize[cnt] + "KB" + "</td>" + 
                    "<td>" + data.svgLog[cnt].numRect + "</td>" + 
                    "<td>" + data.svgLog[cnt].numCirc + "</td>" + 
                    "<td>" + data.svgLog[cnt].numPaths + "</td>" + 
                    "<td>" + data.svgLog[cnt].numGroups + "</td>"
                );

                // add filenames to dropdown list in svg view panel
                if(flag == true) {
                    $("#file-drop-list").append(
                        "<option>" + data.fileName[file] + "</option>"
                    )
                }
                cnt++;
            }
        },

        fail: function(error) {
            alert("File Load Failed");
            console.log(error); 
        }
    });
}

// upload a file to the server
function uploadFile() {
    // upload files and store it to the server
    $("#upload-form").submit(function(e) {
        var filePath = $("#upload").val();
        // console.log("file path: " + filePath);
        var fileDir = filePath.split("\\");
        var fileName = fileDir[fileDir.length-1];
        $("#file-drop-list option").each(function() {
            var temp = $(this).val();
            if(temp == fileName) {
                alert(fileName + " already exists");
                return;
            }
        });
        var ext = fileName.split(".");
        if(ext[1] != "svg") {
            alert("File format invalid");
        }
    });
}

// display svg detail information in the svg view panel
function showSVGInfo() {
    $("#select-attribute").empty();

    $.ajax({
        type: 'get', 
        dataType: 'json', 
        url: '/getSVGInfo', 
        data: {

        },

        success: function(data) {
            var option = $("#file-drop-list option:selected");
            if(option.text() == "-- Select --") {
                $("#svg-view-table").empty();
                $("#component-drop-list").empty();
                $("#component-drop-list").append(
                    "<option>-- Select --</option>"
                );
                $("#select-attribute").empty();
                $("#svg-view-table").append(
                    "<tr> <td colspan='6'> <img id='image'> </td> </tr>" + 
                    "<tr class='table-head'>" + 
                    "<td colspan='3'>Title</td>" + 
                    "<td colspan='3'>Description</td> </tr>" + 
                    "<tr> <td colspan='3'></td>" + 
                    "<td colspan='3'></td> </tr>" + 
                    "<tr class='table-head'>" + 
                    "<td colspan='1'>Component</td>" + 
                    "<td colspan='4'>Summary</td>" + 
                    "<td colspan='1'>Other attributes</td> </tr>" + 
                    "<tr> <td colspan='1'></td>" + 
                    "<td colspan='4'></td>" + 
                    "<td colspan='1'></td> </tr>"
                );
            } else {
                var cnt = 0;
                for(var file in data.fileName) {
                    if(data.fileName[file].search(option.text()) != -1) {
                        break;
                    }
                    cnt++;
                }

                $("#svg-view-table").empty();
                $("#svg-view-table").append(
                    "<tr> <td colspan='6'>" + 
                    "<img src='../uploads/" + option.val() + "?t=" + Math.random() + "' id='image'" + 
                    "</td> </tr>" + 
                    "<tr class='table-head'>" + 
                    "<td colspan='3'>Title</td>" + 
                    "<td colspan='3'>Description</td>" + 
                    "</tr>" + 
                    "<tr>" + 
                    "<td colspan='3' id='title'>" + data.svgTitle[cnt] + "</td>" + 
                    "<td colspan='3' id='description'>" + data.svgDesc[cnt] + "</td>" + 
                    "</tr>" + 
                    "<tr class='table-head'>" + 
                    "<td colspan='1'>Component</td>" + 
                    "<td colspan='4'>Summary</td>" + 
                    "<td colspan='1'>Other attributes</td>" + 
                    "</tr>"
                );

                $("#component-drop-list").empty();
                $("#component-drop-list").append(
                    "<option>-- Select --</option>" + 
                    "<option>SVG Image</option>"
                );

                //rectangle
                var n = 0;
                for(var i = 0; i < cnt; i++) {
                    n += data.numOuterElement[4*i];
                }
                var m = 0;
                for(var i = 0; i < n; i++) {
                    m += data.rectangle[i].numAttr;
                }
                var num = 0;
                var attrStr = "";
                for(var i = 0; i < data.numOuterElement[4*cnt]; i++) {
                    attrStr = "<br> <details> <summary>show</summary> <ul id='show-list'>";
                    for(var j = 0; j < data.rectangle[n].numAttr; j++) {
                        attrStr += "<li>" + data.rectAttr[m].name + ": " + data.rectAttr[m].value + "</li>";
                        m++;
                    }
                    attrStr += "</ul> </summary> </details>";
                    $("#svg-view-table").append(
                        "<tr>" + 
                        "<td colspan='1'>Rectangle " + (i+1) + "</td>" + 
                        "<td colspan='4'>x = " + data.rectangle[n].x + data.rectangle[n].unit + ", " + 
                        "y = " + data.rectangle[n].y + data.rectangle[n].unit + "<br>" + 
                        "Width: " + data.rectangle[n].width + data.rectangle[n].unit + ", " + 
                        "Height: " + data.rectangle[n].height + data.rectangle[n].unit + "</td>" + 
                        "<td colspan='1'>" + data.rectangle[n].numAttr + attrStr + 
                        "</td> </tr>"
                    );
                    n++;
                    num++;
                }
                for(var i = 0; i < num; i++) {
                    $("#component-drop-list").append(
                        "<option>Rectangle " + (i+1) + "</option>"
                    );
                }

                //circle
                n = 0;
                for(var i = 0; i < cnt; i++) {
                    n += data.numOuterElement[4*i+1];
                }
                m = 0;
                for(var i = 0; i < n; i++) {
                    m += data.circle[i].numAttr;
                }
                num = 0;
                for(var i = 0; i < data.numOuterElement[4*cnt+1]; i++) {
                    attrStr = "<br> <details> <summary>show</summary> <ul id='show-list'>";
                    for(var j = 0; j < data.circle[n].numAttr; j++) {
                        attrStr += "<li>" + data.circAttr[m].name + ": " + data.circAttr[m].value + "</li>";
                        m++;
                    }
                    attrStr += "</ul> </summary> </details>";
                    $("#svg-view-table").append(
                        "<tr>" + 
                        "<td colspan='1'>Circle " + (i+1) + "</td>" + 
                        "<td colspan='4'>x = " + data.circle[n].cx + data.circle[n].unit + ", " + 
                        "y = " + data.circle[n].cy + data.circle[n].unit + ", " + 
                        "radius = " + data.circle[n].r + data.circle[n].unit + "</td>" + 
                        "<td colspan='1'>" +  data.circle[n].numAttr + attrStr + 
                        "</td> </tr>"
                    );
                    n++;
                    num++;
                }
                for(var i = 0; i < num; i++) {
                    $("#component-drop-list").append(
                        "<option>Circle " + (i+1) + "</option>"
                    );
                }

                //path
                n = 0;
                for(var i = 0; i < cnt; i++) {
                    n += data.numOuterElement[4*i+2];
                }
                m = 0;
                for(var i = 0; i < n; i++) {
                    m += data.path[i].numAttr;
                }
                num = 0;
                for(var i = 0; i < data.numOuterElement[4*cnt+2]; i++) {
                    attrStr = "<br> <details> <summary>show</summary> <ul id='show-list'>";
                    for(var j = 0; j < data.path[n].numAttr; j++) {
                        attrStr += "<li>" + data.pathAttr[m].name + ": " + data.pathAttr[m].value + "</li>";
                        m++;
                    }
                    attrStr += "</ul> </summary> </details>";
                    $("#svg-view-table").append(
                        "<tr>" + 
                        "<td colspan='1'>Path " + (i+1) + "</td>" + 
                        "<td colspan='4'>path data = " + data.path[n].d + "</td>" + 
                        "<td colspan='1'>" + data.path[n].numAttr + attrStr + 
                        "</td> </tr>"
                    );
                    n++;
                    num++;
                }
                for(var i = 0; i < num; i++) {
                    $("#component-drop-list").append(
                        "<option>Path " + (i+1) + "</option>"
                    );
                }

                //group
                n = 0;
                for(var i = 0; i < cnt; i++) {
                    n += data.numOuterElement[4*i+3];
                }
                m = 0;
                for(var i = 0; i < n; i++) {
                    m += data.group[i].numAttr;
                }
                num = 0;
                for(var i = 0; i < data.numOuterElement[4*cnt+3]; i++) {
                    attrStr = "<br> <details> <summary>show</summary> <ul id='show-list'>";
                    for(var j = 0; j < data.group[n].numAttr; j++) {
                        attrStr += "<li>" + data.groupAttr[m].name + ": " + data.groupAttr[m].value + "</li>";
                        m++;
                    }
                    attrStr += "</ul> </summary> </details>";
                    $("#svg-view-table").append(
                        "<tr>" + 
                        "<td colspan='1'>Group " + (i+1) + "</td>" + 
                        "<td colspan='4'>" + data.group[n].children + " child elements" + "</td>" + 
                        "<td colspan='1'>" + data.group[n].numAttr + attrStr + 
                        "</td> </tr>"
                    );
                    n++;
                    num++;
                }
                for(var i = 0; i < num; i++) {
                    $("#component-drop-list").append(
                        "<option>Group " + (i+1) + "</option>"
                    );
                }
            }
        },

        fail: function(error) {
            alert("File load failed");
            console.log(error); 
        }
    });
}

// edit title
function editTitle() {
    var text = $("#edit-title-text").val();
    var file = $("#file-drop-list").val();
    if(file == "-- Select --") {
        alert("No file selected");
        return;
    } else {
        alert("Title editing completed");
        console.log("Title editing completed");
        $("#edit-title-text").val("");
    }

    $.ajax({
        type: 'get', 
        dataType: 'json', 
        url: '/editTitle', 
        data: {
            fileName: file, 
            title: text
        },

        success: function(data) {
            console.log("Title editing completed");
        },

        fail: function(error) {
            console.log(error); 
        }
    });
}

// edit description
function editDesc() {
    var text = $("#edit-desc-text").val();
    var file = $("#file-drop-list").val();
    if(file == "-- Select --") {
        alert("No file selected");
        return;
    } else {
        alert("Description editing completed");
        console.log("Description editing completed");
        $("#edit-desc-text").val("");
    }

    $.ajax({
        type: 'get', 
        dataType: 'json', 
        url: '/editDesc', 
        data: {
            fileName: file, 
            description: text
        },

        success: function(data) {
            console.log("Description editing completed");
        },

        fail: function(error) {
            console.log(error); 
        }
    });
}

// edit a specific component
function editComponent(str, type) {
    var file = $("#file-drop-list").val();
    if(file == "-- Select --") {
        alert("No file selected");
        return;
    }
    var text;
    var elemType = type;
    var elemIndex;

    //SVG image
    if(type == "SVG_IMAGE") {
        elemIndex = -1;
    } else {
        elemIndex = $("#component-drop-list").val().split(" ")[1] - 1;
    }
    //rectangle
    if(str == "x") {
        text = $("#edit-x-text").val();
        if(text == "") {
            alert("Cannot be empty");
            return;
        } else if(isNumber(text) == false) {
            alert("Invalid input, must be numeric");
            return;
        } else {
            alert("Attribute editing completed");
            console.log("Attribute editing completed");
            $("#edit-x-text").val("");
        }
    }
    else if(str == "y") {
        text = $("#edit-y-text").val();
        if(text == "") {
            alert("Cannot be empty");
            return;
        } else if(isNumber(text) == false) {
            alert("Invalid input, must be numeric");
            return;
        } else {
            alert("Attribute editing completed");
            console.log("Attribute editing completed");
            $("#edit-y-text").val("");
        }
    }
    else if(str == "width") {
        text = $("#edit-width-text").val();
        if(text == "") {
            alert("Cannot be empty");
            return;
        } else if(isNumber(text) == false) {
            alert("Invalid input, must be numeric");
            return;
        } else {
            alert("Attribute editing completed");
            console.log("Attribute editing completed");
            $("#edit-width-text").val("");
        }
    }
    else if(str == "height") {
        text = $("#edit-height-text").val();
        if(text == "") {
            alert("Cannot be empty");
            return;
        } else if(isNumber(text) == false) {
            alert("Invalid input, must be numeric");
            return;
        } else {
            alert("Attribute editing completed");
            console.log("Attribute editing completed");
            $("#edit-height-text").val("");
        }
    }
    //circle
    else if(str == "cx") {
        text = $("#edit-cx-text").val();
        if(text == "") {
            alert("Cannot be empty");
            return;
        } else if(isNumber(text) == false) {
            alert("Invalid input, must be numeric");
            return;
        } else {
            alert("Attribute editing completed");
            console.log("Attribute editing completed");
            $("#edit-cx-text").val("");
        }
    }
    else if(str == "cy") {
        text = $("#edit-cy-text").val();
        if(text == "") {
            alert("Cannot be empty");
            return;
        } else if(isNumber(text) == false) {
            alert("Invalid input, must be numeric");
            return;
        } else {
            alert("Attribute editing completed");
            console.log("Attribute editing completed");
            $("#edit-cy-text").val("");
        }
    }
    else if(str == "r") {
        text = $("#edit-r-text").val();
        if(text == "") {
            alert("Cannot be empty");
            return;
        } else if(isNumber(text) == false) {
            alert("Invalid input, must be numeric");
            return;
        } else {
            alert("Attribute editing completed");
            console.log("Attribute editing completed");
            $("#edit-r-text").val("");
        }
    }
    //path
    else if(str == "d") {
        text = $("#edit-d-text").val();
        if(text == "") {
            alert("Cannot be empty");
            return;
        } else {
            alert("Attribute editing completed");
            console.log("Attribute editing completed");
            $("#edit-d-text").val("");
        }
    }
    //attribute
    else if(str == "attr") {
        str = $("#edit-attrName-text").val();
        if(str == "fill" || str == "viewBox" || str == "enable-background" ||
            str == "opacity" || str == "version" || str == "xmlns" || 
            str == "stroke" || str == "stroke-width" || str == "standalone" || 
            str == "transform" || str == "id" || str == "fill-rule" || 
            str == "stroke-linecap" || str == "stroke-dasharray" || str == "overflow") {
                text = $("#edit-attrVal-text").val();
                alert("Attribute editing completed");
                console.log("Attribute editing completed");
                
        } else {
            alert("Invalid input");
            console.log("Invalid input");
            return;
        }
        $("#edit-attrName-text").val("");
        $("#edit-attrVal-text").val("");
    }

    $.ajax({
        type: 'get', 
        dataType: 'json', 
        url: '/editAttr', 
        data: {
            fileName: file, 
            attrName: str, 
            attrVal: text, 
            type: elemType, 
            index: elemIndex
        },

        success: function(data) {
            alert("Attribute editing completed");
        },

        fail: function(error) {
            alert(error); 
        }
    });
}

// display all components in the drop list
function showComponent() {
    var currentVal = $("#component-drop-list").val().split(" ");
    if(currentVal[0] == "SVG") {
        $("#select-attribute").empty();
        $("#select-attribute").append(
            "<div id='width'>" + 
            "<span class='nametag'>width (cm)</span>" + 
            "<input type='text' class='edit-box' id='edit-width-text'>" + 
            "<input type='submit' value='edit' class='btn submit' id='edit-width' " + 
            "onclick='editComponent(\"width\", \"SVG_IMAGE\");showSVGInfo();'></input>" + 
            "</div>" + 
            "<div id='height'>" + 
            "<span class='nametag'>height (cm)</span>" + 
            "<input type='text' class='edit-box' id='edit-height-text'>" + 
            "<input type='submit' value='edit' class='btn submit' id='edit-height' " + 
            "onclick='editComponent(\"height\", \"SVG_IMAGE\");showSVGInfo();'></input>" + 
            "</div>" + 
            "<div id='attribute'>" + 
            "<span class='nametag'>attribute name</span>" + 
            "<input type='text' class='edit-box edit-attr-box' id='edit-attrName-text'>" + 
            "<span class='nametag' id='attrValue'>value</span>" + 
            "<input type='text' class='edit-box edit-attr-box' id='edit-attrVal-text'>" + 
            "<input type='submit' value='edit/add' class='btn submit' id='edit-attr-text' " + 
            "onclick='editComponent(\"attr\", \"SVG_IMAGE\");showSVGInfo();'></input>" + 
            "</div>"
        );
    }
    else if(currentVal[0] == "Rectangle") {
        $("#select-attribute").empty();
        $("#select-attribute").append(
            "<div id='x'>" + 
            "<span class='nametag'>x</span>" + 
            "<input type='text' class='edit-box' id='edit-x-text'>" + 
            "<input type='submit' value='edit' class='btn submit' id='edit-x' " + 
            "onclick='editComponent(\"x\", \"RECT\");showSVGInfo();'></input>" + 
            "</div>" + 
            "<div id='y'>" + 
            "<span class='nametag'>y</span>" + 
            "<input type='text' class='edit-box' id='edit-y-text'>" + 
            "<input type='submit' value='edit' class='btn submit' id='edit-y' " + 
            "onclick='editComponent(\"y\", \"RECT\");showSVGInfo();'></input>" + 
            "</div>" + 
            "<div id='width'>" + 
            "<span class='nametag'>width</span>" + 
            "<input type='text' class='edit-box' id='edit-width-text'>" + 
            "<input type='submit' value='edit' class='btn submit' id='edit-width' " + 
            "onclick='editComponent(\"width\", \"RECT\");showSVGInfo();'></input>" + 
            "</div>" + 
            "<div id='height'>" + 
            "<span class='nametag'>height</span>" + 
            "<input type='text' class='edit-box' id='edit-height-text'>" + 
            "<input type='submit' value='edit' class='btn submit' id='edit-height' " + 
            "onclick='editComponent(\"height\", \"RECT\");showSVGInfo();'></input>" + 
            "</div>" + 
            "<div id='attribute'>" + 
            "<span class='nametag'>attribute name</span>" + 
            "<input type='text' class='edit-box edit-attr-box' id='edit-attrName-text'>" + 
            "<span class='nametag' id='attrValue'>value</span>" + 
            "<input type='text' class='edit-box edit-attr-box' id='edit-attrVal-text'>" + 
            "<input type='submit' value='edit/add' class='btn submit' id='edit-attr-text' " + 
            "onclick='editComponent(\"attr\", \"RECT\");showSVGInfo();'></input>" + 
            "</div>"
        );
    }
    else if(currentVal[0] == "Circle") {
        $("#select-attribute").empty();
        $("#select-attribute").append(
            "<div id='cx'>" + 
            "<span class='nametag'>x</span>" + 
            "<input type='text' class='edit-box' id='edit-cx-text'>" + 
            "<input type='submit' value='edit' class='btn submit' id='edit-cx' " + 
            "onclick='editComponent(\"cx\", \"CIRC\");showSVGInfo();'></input>" + 
            "</div>" + 
            "<div id='cy'>" + 
            "<span class='nametag'>y</span>" + 
            "<input type='text' class='edit-box' id='edit-cy-text'>" + 
            "<input type='submit' value='edit' class='btn submit' id='edit-cy' " + 
            "onclick='editComponent(\"cy\", \"CIRC\");showSVGInfo();'></input>" + 
            "</div>" + 
            "<div id='r'>" + 
            "<span class='nametag'>radius</span>" + 
            "<input type='text' class='edit-box' id='edit-r-text'>" + 
            "<input type='submit' value='edit' class='btn submit' id='edit-r' " + 
            "onclick='editComponent(\"r\", \"CIRC\");showSVGInfo();'></input>" + 
            "</div>" + 
            "<div id='attribute'>" + 
            "<span class='nametag'>attribute name</span>" + 
            "<input type='text' class='edit-box edit-attr-box' id='edit-attrName-text'>" + 
            "<span class='nametag' id='attrValue'>value</span>" + 
            "<input type='text' class='edit-box edit-attr-box' id='edit-attrVal-text'>" + 
            "<input type='submit' value='edit/add' class='btn submit' id='edit-attr-text' " + 
            "onclick='editComponent(\"attr\", \"CIRC\");showSVGInfo();'></input>" + 
            "</div>"
        );
    }
    else if(currentVal[0] == "Path") {
        $("#select-attribute").empty();
        $("#select-attribute").append(
            "<div id='d'>" + 
            "<span class='nametag'>path data</span>" + 
            "<input type='text' class='edit-box' id='edit-d-text'>" + 
            "<input type='submit' value='edit' class='btn submit' id='edit-d' " + 
            "onclick='editComponent(\"d\", \"PATH\");showSVGInfo();'></input>" + 
            "</div>" + 
            "<div id='attribute'>" + 
            "<span class='nametag'>attribute name</span>" + 
            "<input type='text' class='edit-box edit-attr-box' id='edit-attrName-text'>" + 
            "<span class='nametag' id='attrValue'>value</span>" + 
            "<input type='text' class='edit-box edit-attr-box' id='edit-attrVal-text'>" + 
            "<input type='submit' value='edit/add' class='btn submit' id='edit-attr-text' " + 
            "onclick='editComponent(\"attr\", \"PATH\");showSVGInfo();'></input>" + 
            "</div>"
        );
    }
    else if(currentVal[0] == "Group") {
        $("#select-attribute").empty();
        $("#select-attribute").append(
        "<div id='attribute'>" + 
            "<span class='nametag'>attribute name</span>" + 
            "<input type='text' class='edit-box edit-attr-box' id='edit-attrName-text'>" + 
            "<span class='nametag' id='attrValue'>value</span>" + 
            "<input type='text' class='edit-box edit-attr-box' id='edit-attrVal-text'>" + 
            "<input type='submit' value='edit/add' class='btn submit' id='edit-attr-text' " + 
            "onclick='editComponent(\"attr\", \"GROUP\");showSVGInfo();'></input>" + 
            "</div>"
        );
    }
    else {
        $("#select-attribute").empty();
    }
}

// display add shape section
function showAddShape() {
    var shape = $("#shape-list").val();
    if(shape == "-- Select --") {
        $("#shape-attribute").empty();
    }
    else if(shape == "Rectangle") {
        $("#shape-attribute").empty();
        $("#shape-attribute").append(
            "<div id='rect-x'>" + 
            "<span class='nametag'>x</span>" + 
            "<input type='text' class='edit-box' id='add-x-text'>" + 
            "</div>" + 
            "<div id='rect-y'>" + 
            "<span class='nametag'>y</span>" + 
            "<input type='text' class='edit-box' id='add-y-text'>" + 
            "</div>" + 
            "<div id='rect-width'>" + 
            "<span class='nametag'>width</span>" + 
            "<input type='text' class='edit-box' id='add-width-text'>" + 
            "</div>" + 
            "<div id='rect-height'>" + 
            "<span class='nametag'>height</span>" + 
            "<input type='text' class='edit-box' id='add-height-text'>" + 
            "</div>" + 
            "<div id='unit'>" + 
            "<span class='nametag'>unit</span>" + 
            "<input type='text' class='edit-box' id='add-unit-text'>" + 
            "</div>" + 
            "<div id='rect-attribute'>" + 
            "<span class='nametag'>attribute name</span>" + 
            "<input type='text' class='edit-box edit-attr-box' id='add-attrName-text'>" + 
            "<span class='nametag'>value</span>" + 
            "<input type='text' class='edit-box edit-attr-box' id='add-attrVal-text'>" + 
            "</div>" + 
            "<div id='add'>" + 
            "<input type='submit' value='Add Shape' class='btn submit add-shape-btn' id='add-rect-btn' " + 
            "onclick='addShape(\"RECT\");loadAllFiles(false);showSVGInfo();'>" + 
            "</div>"
        );
    }
    else if(shape == "Circle") {
        $("#shape-attribute").empty();
        $("#shape-attribute").append(
            "<div id='circ-cx'>" + 
            "<span class='nametag'>x</span>" + 
            "<input type='text' class='edit-box' id='add-cx-text'>" + 
            "</div>" + 
            "<div id='circ-cy'>" + 
            "<span class='nametag'>y</span>" + 
            "<input type='text' class='edit-box' id='add-cy-text'>" + 
            "</div>" + 
            "<div id='circ-r'>" + 
            "<span class='nametag'>radius</span>" + 
            "<input type='text' class='edit-box' id='add-r-text'>" + 
            "</div>" + 
            "<div id='unit'>" + 
            "<span class='nametag'>unit</span>" + 
            "<input type='text' class='edit-box' id='add-unit-text'>" + 
            "</div>" + 
            "<div id='circ-attribute'>" + 
            "<span class='nametag'>attribute name</span>" + 
            "<input type='text' class='edit-box edit-attr-box' id='add-attrName-text'>" + 
            "<span class='nametag'>value</span>" + 
            "<input type='text' class='edit-box edit-attr-box' id='add-attrVal-text'>" + 
            "</div>" + 
            "<div id='add'>" + 
            "<input type='submit' value='Add Shape' class='btn submit add-shape-btn' id='add-circ-btn'" + 
            "onclick='addShape(\"CIRC\");loadAllFiles(false);showSVGInfo();'>" + 
            "</div>"
        );
    }
}

// add shape to existing svg file
function addShape(type) {
    var file = $("#file-drop-list").val();
    if(file == "-- Select --") {
        alert("No file selected");
        return;
    }

    var shapeAttrs = new Array();
    if(type == "RECT") {
        var rect = {};
        rect.x = $("#add-x-text").val();
        rect.y = $("#add-y-text").val();
        rect.width = $("#add-width-text").val();
        rect.height = $("#add-height-text").val();
        rect.unit = $("#add-unit-text").val();
        rect.attrName = $("#add-attrName-text").val();
        rect.attrVal = $("#add-attrVal-text").val();
        if(rect.x == "" || rect.y == "" || rect.width == "" || rect.height == "") {
            alert("x/y/width/height cannot be empty");
            console.log("x/y/width/height cannot be empty");
            return;
        }
        shapeAttrs.push(rect);
        alert("Add shape completed");
        console.log("Add shape completed");
        $("#add-x-text").val("");
        $("#add-y-text").val("");
        $("#add-width-text").val("");
        $("#add-height-text").val("");
        $("#add-unit-text").val("");
        $("#add-attrName-text").val("");
        $("#add-attrVal-text").val("");
        $("#shape-list").val("-- Select --");
        $("#shape-attribute").empty();
    }
    else if(type == "CIRC") {
        var circ = {};
        circ.cx = $("#add-cx-text").val();
        circ.cy = $("#add-cy-text").val();
        circ.r = $("#add-r-text").val();
        circ.unit = $("#add-unit-text").val();
        circ.attrName = $("#add-attrName-text").val();
        circ.attrVal = $("#add-attrVal-text").val();
        if(circ.cx == "" || circ.cy == "" || circ.r == "") {
            alert("x/y/radius cannot be empty");
            console.log("x/y/radius cannot be emptyt");
            return;
        }
        shapeAttrs.push(circ);
        alert("Add shape completed");
        console.log("Add shape completed");
        $("#add-cx-text").val("");
        $("#add-cy-text").val("");
        $("#add-r-text").val("");
        $("#add-unit-text").val("");
        $("#add-attrName-text").val("");
        $("#add-attrVal-text").val("");
        $("#shape-list").val("-- Select --");
        $("#shape-attribute").empty();
    }

    $.ajax({
        type: 'get', 
        dataType: 'json', 
        url: '/addShape', 
        data: {
            fileName: file, 
            shapeType: type, 
            shapeAttr: shapeAttrs
        },

        success: function(data) {
            alert("Add shape completed");
            console.log("Add shape completed");
        },

        fail: function(error) {
            alert(error); 
        }
    });
}

// create a new svg file
function createSVG() {
    var filename = $("#filename-text").val();
    var titleText = $("#title-text").val();
    var descText = $("#description-text").val();
    if(filename == "") {
        alert("File name cannot be empty");
        return;
    } else if(filename.search(".svg") == -1) {
        filename += ".svg";
    }
    alert("SVG successful created");
    console.log("SVG successful created");
    $("#filename-text").val("");
    $("#title-text").val("");
    $("#description-text").val("");

    $.ajax({
        type: 'get', 
        dataType: 'json', 
        url: '/createSVG', 
        data: {
            fileName: filename, 
            title: titleText, 
            description: descText
        },

        success: function(data) {
            alert("Create SVG image completed");
            console.log("Create SVG image completed");
        },

        fail: function(error) {
            alert(error); 
        }
    });

    loadAllFiles(true);
    showSVGInfo();
}

// scale all rectangles or circles in the image by the same factor
function scaleShapes() {
    var filename = $("#file-drop-list").val();
    var shape = $("input[name='scale-shape']:checked").val();
    var factor = $("input[name='scale-factor']:checked").val();
    if(shape != "rectangles" && shape != "circles") {
        alert("Shape must be selected");
        console.log("Shape must be selected");
        $("#scale-text").val("");
        return;
    }
    if(factor != "0.5" && factor != "1.0" && factor != "1.5" && 
        factor != "2.0" && factor != "3.0" && factor != "4.0" &&
        factor != "other") {
            alert("Scale factor must be selected");
            console.log("Scale factor must be selected");
            $("#scale-text").val("");
            return;
    }
    if(factor == "other") {
        factor = $("#scale-text").val();
        if(factor[factor.length-1] == "x") {
            factor = factor.substr(0, factor.length-1);
        }
    }
    if(factor <= 0 || factor > 999) {
        alert("Scale factor out of range");
        console.log("Scale factor out of range");
        $("#scale-text").val("");
        return;
    }

    alert("Scale shape completed");
    console.log("Scale shape completed");
    $("#scale-text").val("");

    $.ajax({
        type: 'get', 
        dataType: 'json', 
        url: '/scaleShapes', 
        data: {
            fileName: filename, 
            scaleShape: shape, 
            scaleFactor: factor
        },

        success: function(data) {
            alert("Scale shape completed");
            console.log("Scale shape completed");
        },

        fail: function(error) {
            alert(error); 
        }
    });
}