'use strict'

// C library API
const ffi = require('ffi-napi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());
app.use(express.static(path.join(__dirname+'/uploads')));

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
    res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
    res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
    fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
        const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
        res.contentType('application/javascript');
        res.send(minimizedContents._obfuscatedCode);
    });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
    if(!req.files) {
        return res.status(400).send('No files were uploaded.');
    }
 
	let uploadFile = req.files.uploadFile;
	if(uploadFile.name.search(".svg") != -1) {
		var folder = "./uploads/";
		var flag = false;
		fs.readdir(folder, (err, files) => {
			if(err) {
				console.error("File open failed");
			} else {
				files.forEach(file => {
					if(file == uploadFile.name) {
						console.log(uploadFile.name + " already exists");
						flag = true;
					}
				});
			}
		});
		if(flag == false) {
			// Use the mv() method to place the file somewhere on your server
			uploadFile.mv('uploads/' + uploadFile.name, function(err) {
				if(err) {
					return res.status(500).send(err);
				}
				res.redirect('/');
			});
		}
	} else {
		console.log(uploadFile.name + " format invalid");
		res.redirect('/');
	}
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
    fs.stat('uploads/' + req.params.name, function(err, stat) {
        if(err == null) {
          res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
        } else {
          console.log('Error in file downloading route: '+err);
          res.send('');
        }
    });
});

//******************** Your code goes here ********************

// required C functions
let libsvgparse = ffi.Library('./libsvgparse', {
	"createValidSVGimage" : ["pointer", ["string", "string"] ], 
	"deleteSVGimage" : ["pointer", [] ], 
	"SVGtoJSON" : ["string", ["pointer"] ], 
	"getFileSize" : ["int", ["string"] ], 
	"titleToJSON" : ["string", ["pointer"] ], 
	"descToJSON" : ["string", ["pointer"] ], 
	"countOuterRects" : ["int", ["pointer"] ], 
	"countOuterCircles" : ["int", ["pointer"] ], 
	"countOuterPaths" : ["int", ["pointer"] ], 
	"countOuterGroups" : ["int", ["pointer"] ], 
	"getRectsInfo" : ["string", ["pointer"] ], 
	"getCirclesInfo" : ["string", ["pointer"] ], 
	"getPathsInfo" : ["string", ["pointer"] ], 
	"getGroupsInfo" : ["string", ["pointer"] ], 
	"getRectAttrInfo" : ["string", ["pointer"] ], 
	"getCircAttrInfo" : ["string", ["pointer"] ], 
	"getPathAttrInfo" : ["string", ["pointer"] ], 
	"getGroupAttrInfo" : ["string", ["pointer"] ], 
	"editTitle" : ["int", ["string", "string"] ], 
	"editDesc" : ["int", ["string", "string"] ], 
	"editAttribute" : ["int", ["string", "string", "string", "string", "int"]], 
	"addRect" : ["int", ["string", "float", "float", "float", "float", "string", "string", "string"] ], 
	"addCirc" : ["int", ["string", "float", "float", "float", "string", "string", "string"] ], 
	"createNewSVGImage" : ["void", ["string", "string", "string"] ], 
	"scaleShapes" : ["int", ["string", "string", "float"] ]
});

// get all files from the server
app.get('/getFiles', function(req , res) {	
	var folder = './uploads/';
	var fileNames = new Array();
	var svgLogs = new Array();
	var size = new Array();

	fs.readdir(folder, (err, files) => {
		if(err) {
			console.error("File open failed");
		}

		files.forEach(file => {
			if(file.indexOf(".svg") == -1) {
				console.log(file + " failed to create SVG file - invalid format");
			} else {
				console.log(file + " successful uploaded");
				fileNames.push(file);
				var svg = libsvgparse.createValidSVGimage(folder+file, "schemaFile");
				var fileLog = libsvgparse.SVGtoJSON(svg);
				var svgObj = JSON.parse(fileLog);
				var sizeKB = libsvgparse.getFileSize(folder + file);
				size.push(sizeKB);
				var svgImage = {};
				svgImage.numRect = svgObj["numRect"];
				svgImage.numCirc = svgObj["numCirc"];
				svgImage.numPaths = svgObj["numPaths"];
				svgImage.numGroups = svgObj["numGroups"];
				svgLogs.push(svgImage);
			}
		});
	  
		res.send({
			fileName: fileNames, 
			svgLog: svgLogs, 
			fileSize: size
		});
	});
});

// get all svg files information
app.get('/getSVGInfo', function(req , res) {
	var folder = './uploads/';
	var fileNames = new Array();
	var titles = new Array();
	var descriptions = new Array();
	var outerElements = new Array();
	var rectangles = new Array();
	var circles = new Array();
	var paths = new Array();
	var groups = new Array();
	var rectAttrs = new Array();
	var circAttrs = new Array();
	var pathAttrs = new Array();
	var groupAttrs = new Array();

	fs.readdir(folder, (err, files) => {
		if(err) {
			console.error("File open failed");
		}

		files.forEach(file => {
			if(file.indexOf(".svg") == -1) {
				console.log(file + " failed to create SVG file - invalid format");
			} else {
				console.log(file + " successful uploaded");
				fileNames.push(file);
				var svg = libsvgparse.createValidSVGimage(folder+file, "schemaFile");
				var title = libsvgparse.titleToJSON(svg);
				titles.push(title);
				var desc = libsvgparse.descToJSON(svg);
				descriptions.push(desc);
				// get all immediate children elements
				var outerRects = libsvgparse.countOuterRects(svg);
				outerElements.push(outerRects);
				var outerCircles = libsvgparse.countOuterCircles(svg);
				outerElements.push(outerCircles);
				var outerPaths = libsvgparse.countOuterPaths(svg);
				outerElements.push(outerPaths);
				var outerGroups = libsvgparse.countOuterGroups(svg);
				outerElements.push(outerGroups);

				// get element summary information
				//rectangle
				var rectInfo = libsvgparse.getRectsInfo(svg);
				var rectJSON = JSON.parse(rectInfo);
				for(var r in rectJSON) {
					var rect = {};
					rect.x = rectJSON[r]["x"];
					rect.y = rectJSON[r]["y"];
					rect.width = rectJSON[r]["w"];
					rect.height = rectJSON[r]["h"];
					rect.numAttr = rectJSON[r]["numAttr"];
					rect.unit = rectJSON[r]["units"];
					rectangles.push(rect);
				}
				var rectAttrInfo = libsvgparse.getRectAttrInfo(svg);
				var rectAttrJSON = JSON.parse(rectAttrInfo);
				for(var a in rectAttrJSON) {
					if(typeof(rectAttrJSON[a]["name"]) == "undefined" || typeof(rectAttrJSON[a]["value"]) == "undefined") {
						console.log("skip empty attribute")
					} else {
						var attr = {};
						attr.name = rectAttrJSON[a]["name"];
						attr.value = rectAttrJSON[a]["value"];
						rectAttrs.push(attr);
					}
				}

				//circle
				var circInfo = libsvgparse.getCirclesInfo(svg);
				var circJSON = JSON.parse(circInfo);
				for(var c in circJSON) {
					var circ = {};
					circ.cx = circJSON[c]["cx"];
					circ.cy = circJSON[c]["cy"];
					circ.r = circJSON[c]["r"];
					circ.numAttr = circJSON[c]["numAttr"];
					circ.unit = circJSON[c]["units"];
					circles.push(circ);
				}
				var circAttrInfo = libsvgparse.getCircAttrInfo(svg);
				var circAttrJSON = JSON.parse(circAttrInfo);
				for(var a in circAttrJSON) {
					if(typeof(circAttrJSON[a]["name"]) == "undefined" || typeof(circAttrJSON[a]["value"]) == "undefined") {
						console.log("skip empty attribute")
					} else {
						var attr = {};
						attr.name = circAttrJSON[a]["name"];
						attr.value = circAttrJSON[a]["value"];
						circAttrs.push(attr);
					}
				}

				//path
				var pathInfo = libsvgparse.getPathsInfo(svg);
				var pathJSON = JSON.parse(pathInfo);
				for(var p in pathJSON) {
					var path = {};
					path.d = pathJSON[p]["d"];
					path.numAttr = pathJSON[p]["numAttr"];
					paths.push(path);
				}
				var pathAttrInfo = libsvgparse.getPathAttrInfo(svg);
				var pathAttrJSON = JSON.parse(pathAttrInfo);
				for(var a in pathAttrJSON) {
					if(typeof(pathAttrJSON[a]["name"]) == "undefined" || typeof(pathAttrJSON[a]["value"]) == "undefined") {
						console.log("skip empty attribute")
					} else {
						var attr = {};
						attr.name = pathAttrJSON[a]["name"];
						attr.value = pathAttrJSON[a]["value"];
						pathAttrs.push(attr);
					}
				}

				//group
				var groupInfo = libsvgparse.getGroupsInfo(svg);
				var groupJSON = JSON.parse(groupInfo);
				for(var g in groupJSON) {
					var group = {};
					group.children = groupJSON[g]["children"];
					group.numAttr = groupJSON[g]["numAttr"];
					groups.push(group);
				}
				var groupAttrInfo = libsvgparse.getGroupAttrInfo(svg);
				var groupAttrJSON = JSON.parse(groupAttrInfo);
				for(var a in groupAttrJSON) {
					if(typeof(groupAttrJSON[a]["name"]) == "undefined" || typeof(groupAttrJSON[a]["value"]) == "undefined") {
						console.log("skip empty attribute")
					} else {
						var attr = {};
						attr.name = groupAttrJSON[a]["name"];
						attr.value = groupAttrJSON[a]["value"];
						groupAttrs.push(attr);
					}
				}
			}
		});
	  
		res.send({
			fileName: fileNames, 
			svgTitle: titles, 
			svgDesc: descriptions, 
			numOuterElement: outerElements, 
			rectangle: rectangles, 
			circle: circles, 
			path: paths, 
			group: groups, 
			rectAttr: rectAttrs, 
			circAttr: circAttrs, 
			pathAttr: pathAttrs, 
			groupAttr: groupAttrs
		});
	});
});

app.get('/editTitle', function(req, res) {
	var fileName = "./uploads/" + req.query.fileName;
	var title = req.query.title;
	var flag = libsvgparse.editTitle(fileName, title);
	if(flag == 1) {
		console.log("Title successful edited");
	} else {
		console.log("Title editing failed");
	}
});

app.get('/editDesc', function(req, res) {
	var fileName = "./uploads/" + req.query.fileName;
	var desc = req.query.description;
	var flag = libsvgparse.editDesc(fileName, desc);
	if(flag == 1) {
		console.log("Description successful edited");
	} else {
		console.log("Description editing failed");
	}
});

app.get('/editAttr', function(req, res) {
	var fileName = "./uploads/" + req.query.fileName;
	var attrName = req.query.attrName;
	var attrVal = req.query.attrVal;
	var type = req.query.type;
	var index = req.query.index;
	var flag = libsvgparse.editAttribute(fileName, attrName, attrVal, type, index);
	if(flag == 1) {
		console.log(attrName + " successful edited");
	} else {
		console.log(attrName + " editing failed");
	}
});

app.get('/addShape', function(req, res) {
	var fileName = "./uploads/" + req.query.fileName;
	var type = req.query.shapeType;
	if(type == "RECT") {
		var attrObj;
		for(var obj in req.query.shapeAttr) {
			attrObj = req.query.shapeAttr[obj];
		}
		var x = attrObj["x"];
		var y = attrObj["y"];
		var width = attrObj["width"];
		var height = attrObj["height"];
		var unit = attrObj["unit"];
		var attrName = attrObj["attrName"];
		var attrVal = attrObj["attrVal"];
		var flag = libsvgparse.addRect(fileName, x, y, width, height, unit, attrName, attrVal);
		if(flag == 1) {
			console.log("Rectangle successful added");
		} else {
			console.log("Rectangle add failed");
		}
	}
	else if(type == "CIRC") {
		var attrObj;
		for(var obj in req.query.shapeAttr) {
			attrObj = req.query.shapeAttr[obj];
		}
		var cx = attrObj["cx"];
		var cy = attrObj["cy"];
		var r = attrObj["r"];
		var unit = attrObj["unit"];
		var attrName = attrObj["attrName"];
		var attrVal = attrObj["attrVal"];
		var flag = libsvgparse.addCirc(fileName, cx, cy, r, unit, attrName, attrVal);
		if(flag == 1) {
			console.log("Circle successful added");
		} else {
			console.log("Circle add failed");
		}
	}
});

app.get('/createSVG', function(req, res) {
	var fileName = "./uploads/" + req.query.fileName;
	var title =  req.query.title;
	var description =  req.query.description;
	libsvgparse.createNewSVGImage(fileName, title, description);
});

app.get('/scaleShapes', function(req, res) {
	var fileName = "./uploads/" + req.query.fileName;
	var shape =  req.query.scaleShape;
	var factor =  req.query.scaleFactor;
	libsvgparse.scaleShapes(fileName, shape, factor);
});

app.listen(portNum);
console.log("Running app at localhost: " + portNum);