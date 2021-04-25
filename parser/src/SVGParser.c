#include "SVGParser.h"
#include "Helper.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

char* extractContent(xmlNode *node) {
	if(node->children != NULL && node->children->content != NULL) {
		return (char *)node->children->content;
	}
	return NULL;
}

Rectangle* extractRectangle(xmlNode *node) {
	Rectangle *rect = (Rectangle *)malloc(sizeof(Rectangle));
	rect->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
	xmlAttr *attr;
	for(attr = node->properties; attr != NULL; attr = attr->next) {
		xmlNode *value = attr->children;
		char *attrName = (char *)attr->name;
		char *cont = (char *)value->content;
		float temp;
		strcpy(rect->units, "");
		sscanf(cont, "%f%s", &temp, rect->units);
		if(strcmp(attrName, "x") == 0) {
			rect->x = atof(cont);
		} else if(strcmp(attrName, "y") == 0) {
			rect->y = atof(cont);
		} else if(strcmp(attrName, "width") == 0) {
			rect->width = atof(cont);
		} else if(strcmp(attrName, "height") == 0) {
			rect->height = atof(cont);
		} else {
			Attribute *a = (Attribute *)malloc(sizeof(Attribute));
			a->name = (char *)malloc(sizeof(char) * 1024);
			a->value = (char *)malloc(sizeof(char) * 1024 * 32);
			strcpy(a->name, attrName);
			strcpy(a->value, cont);
			insertBack(rect->otherAttributes, (void *)a);
		}
	}
	return rect;
}

Circle* extractCircle(xmlNode *node) {
	Circle *circle = (Circle *)malloc(sizeof(Circle));
	circle->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
	xmlAttr *attr;
	for(attr = node->properties; attr != NULL; attr = attr->next) {
		xmlNode *value = attr->children;
		char *attrName = (char *)attr->name;
		char *cont = (char *)value->content;
		float temp;
		strcpy(circle->units, "");
		sscanf(cont, "%f%s", &temp, circle->units);
		if(strcmp(attrName, "cx") == 0) {
			circle->cx = atof(cont);
		} else if(strcmp(attrName, "cy") == 0) {
			circle->cy = atof(cont);
		} else if(strcmp(attrName, "r") == 0) {
			circle->r = atof(cont);
		} else {
			Attribute *a = (Attribute *)malloc(sizeof(Attribute));
			a->name = (char *)malloc(sizeof(char) * 1024);
			a->value = (char *)malloc(sizeof(char) * 1024 * 32);
			strcpy(a->name, attrName);
			strcpy(a->value, cont);
			insertBack(circle->otherAttributes, (void *)a);
		}
	}
	return circle;
}

Path* extractPath(xmlNode *node) {
	Path *path = (Path *)malloc(sizeof(Rectangle));
	path->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
	xmlAttr *attr;
	for(attr = node->properties; attr != NULL; attr = attr->next) {
		xmlNode *value = attr->children;
		char *attrName = (char *)attr->name;
		char *cont = (char *)value->content;
		if(strcmp(attrName, "d") == 0) {
			path->data = (char *)malloc(sizeof(char) * strlen(cont) + 1);
			strcpy(path->data, cont);
		} else {
			Attribute *a = (Attribute *)malloc(sizeof(Attribute));
			a->name = (char *)malloc(sizeof(char) * 1024);
			a->value = (char *)malloc(sizeof(char) * 1024 * 32);
			strcpy(a->name, attrName);
			strcpy(a->value, cont);
			insertBack(path->otherAttributes, (void *)a);
		}
	}
	return path;
}

Group* extractGroup(xmlNode *node, SVGimage *img) {
	Group *group = (Group *)malloc(sizeof(Group));
	group->rectangles = initializeList(&rectangleToString, &deleteRectangle, &compareRectangles);
	group->circles = initializeList(&circleToString, &deleteCircle, &compareCircles);
	group->paths = initializeList(&pathToString, &deletePath, &comparePaths);
	group->groups = initializeList(&groupToString, &deleteGroup, &compareGroups);
	group->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);

	xmlAttr *attr;
	for(attr = node->properties; attr != NULL; attr = attr->next) {
		xmlNode *value = attr->children;
		char *attrName = (char *)attr->name;
		char *cont = (char *)value->content;
		Attribute *a = (Attribute *)malloc(sizeof(Attribute));
		a->name = (char *)malloc(sizeof(char) * 1024);
		a->value = (char *)malloc(sizeof(char) * 1024 * 32);
		strcpy(a->name, attrName);
		strcpy(a->value, cont);
		insertBack(group->otherAttributes, (void *)a);
	}

	xmlNode *cur_node = NULL;
	for(cur_node = node->children; cur_node != NULL; cur_node = cur_node->next) {
		if(cur_node->type == XML_ELEMENT_NODE) {
			// extract rect
			if(strcmp((const char *)cur_node->name, "rect") == 0) {
				Rectangle *rect = extractRectangle(cur_node);
				insertBack(group->rectangles, (void *)rect);
			}
			// extract circle
			else if(strcmp((const char *)cur_node->name, "circle") == 0) {
				Circle *circle = extractCircle(cur_node);
				insertBack(group->circles, (void *)circle);
			}
			// extract path
			else if(strcmp((const char *)cur_node->name, "path") == 0) {
				Path *path = extractPath(cur_node);
				insertBack(group->paths, (void *)path);
			}
			// extract group
			else if(strcmp((const char *)cur_node->name, "g") == 0) {
				Group *g = extractGroup(cur_node, img);
				insertBack(group->groups, (void *)g);
			}
		}
	}
	return group;
}

/**
 * Function to create an SVG object based on the contents of an SVG file.
 * @pre File name cannot be an empty string or NULL.
        File represented by this name must exist and must be readable.
 * @post Either:
         A valid SVGimage has been created and its address was returned
		 or
		 An error occurred, and NULL was returned
 * @return the pinter to the new struct or NULL
 * @param fileName - a string containing the name of the SVG file
 */
SVGimage* createSVGimage(char *fileName) {
	if(fileName == NULL || strlen(fileName) == 0) {
		return NULL;
	}

	xmlDoc *doc = NULL;
	xmlNode *root_element = NULL;
	xmlNode *cur_node = NULL;

	LIBXML_TEST_VERSION

	doc = xmlReadFile(fileName, NULL, 0);
	if(doc == NULL) {
		return NULL;
	}

	SVGimage *img = (SVGimage *)malloc(sizeof(SVGimage));
	img->rectangles = initializeList(&rectangleToString, &deleteRectangle, &compareRectangles);
	img->circles = initializeList(&circleToString, &deleteCircle, &compareCircles);
	img->paths = initializeList(&pathToString, &deletePath, &comparePaths);
	img->groups = initializeList(&groupToString, &deleteGroup, &compareGroups);
	img->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);

	root_element = xmlDocGetRootElement(doc);

	xmlAttr *attr;
	for(attr = root_element->properties; attr != NULL; attr = attr->next) {
		xmlNode *value = attr->children;
		char *attrName = (char *)attr->name;
		char *cont = (char *)value->content;
		Attribute *a = (Attribute*) malloc(sizeof(Attribute));
		a->name = (char *)malloc(sizeof(char) * 1024);
		a->value = (char *)malloc(sizeof(char) * 1024 * 32);
		strcpy(a->name, attrName);
		strcpy(a->value, cont);
		insertBack(img->otherAttributes, (void *)a);
	}

	strcpy(img->namespace, "");
	strcpy(img->title, "");
	strcpy(img->description, "");

	// extract namespace
	if(strcmp((char *)root_element->name, "svg") == 0) {
		strcpy(img->namespace, (char *)root_element->ns->href);
	}

	for(cur_node = root_element->children; cur_node != NULL; cur_node = cur_node->next) {
		if(cur_node->type == XML_ELEMENT_NODE) {
			// extract group
			if(strcmp((const char *)cur_node->name, "g") == 0) {
				Group *group = extractGroup(cur_node, img);
				insertBack(img->groups, (void *)group);
			}

			// extract title
			if(strcmp((const char *)cur_node->name, "title") == 0) {
				strcpy(img->title, extractContent(cur_node));
			}
			// extract description
			else if(strcmp((const char *)cur_node->name, "desc") == 0) {
				strcpy(img->description, extractContent(cur_node));
			}

			if(strcmp((const char *)cur_node->parent->name, "g") != 0) {
				// extract rect
				if(strcmp((const char *)cur_node->name, "rect") == 0) {
					Rectangle *rect = extractRectangle(cur_node);
					insertBack(img->rectangles, (void *)rect);
				}
				// extract circle
				else if(strcmp((const char *)cur_node->name, "circle") == 0) {
					Circle *circle = extractCircle(cur_node);
					insertBack(img->circles, (void *)circle);
				}
				// extract path
				else if(strcmp((const char *)cur_node->name, "path") == 0) {
					Path *path = extractPath(cur_node);
					insertBack(img->paths, (void *)path);
				}
			}
		}
	}

	xmlFreeDoc(doc);
	xmlCleanupParser();
	return img;
}

/**
 * Function to create a string representation of an SVG object.
 * @pre SVGimgage exists, is not null, and is valid
 * @post SVGimgage has not been modified in any way, and a string representing the SVG contents has been created
 * @return a string contaning a humanly readable representation of an SVG object
 * @param obj - a pointer to an SVG struct
 */
char* SVGimageToString(SVGimage *img) {
	return NULL;
}

/**
 * Function to delete image content and free all the memory.
 * @pre SVGimgage  exists, is not null, and has not been freed
 * @post SVSVGimgageG  had been freed
 * @return none
 * @param obj - a pointer to an SVG struct
 */
void deleteSVGimage(SVGimage *img) {
	if(img == NULL) {
		return;
	}
	freeList(img->rectangles);
	freeList(img->circles);
	freeList(img->paths);
	freeList(img->groups);
	freeList(img->otherAttributes);
	free(img);
}

/**
 * For the four "get..." functions below, make sure you return a list of opinters to the existing structs
 * - do not allocate new structs.  They all share the same format, and only differ in the contents of the lists
 * they return.
 * 
 * @pre SVGimgage exists, is not null, and has not been freed
 * @post SVGimgage has not been modified in any way
 * @return a newly allocated List of components.  While the List struct itself is new, the components in it are just pointers
 * to the ones in the image.
 * 
 * The list must me empty if the element is not found - do not return NULL
 * 
 * @param obj - a pointer to an SVG struct
 */

void getInnerRects(List *rectList, Group *group) {
	if(getLength(group->groups) > 0) {
		ListIterator iter = createIterator(group->groups);
		void *elem;
		while((elem = nextElement(&iter)) != NULL) {
			Group *g = (Group *)elem;
			getInnerRects(rectList, g);
		}
	}

	ListIterator iter = createIterator(group->rectangles);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		Rectangle *rect = (Rectangle *)elem;
		insertBack(rectList, (void *)rect);
	}
}

// Function that returns a list of all rectangles in the image.
List* getRects(SVGimage *img) {
	if(img == NULL) {
		return NULL;
	}

	List *rectAll = initializeList(&rectangleToString, &deleteRectangle, &compareRectangles);

	ListIterator iter = createIterator(img->rectangles);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		Rectangle *rect = (Rectangle *)elem;
		insertBack(rectAll, (void *)rect);
	}

	iter = createIterator(img->groups);
	while((elem = nextElement(&iter)) != NULL) {
		Group *group = (Group *)elem;
		getInnerRects(rectAll, group);
	}
	return rectAll;
}

void getInnerCircles(List *circleList, Group *group) {
	if(getLength(group->groups) > 0) {
		ListIterator iter = createIterator(group->groups);
		void *elem;
		while((elem = nextElement(&iter)) != NULL) {
			Group *g = (Group *)elem;
			getInnerCircles(circleList, g);
		}
	}

	ListIterator iter = createIterator(group->circles);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		Circle *circle = (Circle *)elem;
		insertBack(circleList, (void *)circle);
	}
}

// Function that returns a list of all circles in the image.
List* getCircles(SVGimage *img) {
	if(img == NULL) {
		return NULL;
	}

	List *circleAll = initializeList(&circleToString, &deleteCircle, &compareCircles);
	
	ListIterator iter = createIterator(img->circles);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		Circle *circle = (Circle *)elem;
		insertBack(circleAll, (void *)circle);
	}

	iter = createIterator(img->groups);
	while((elem = nextElement(&iter)) != NULL) {
		Group *group = (Group *)elem;
		getInnerCircles(circleAll, group);
	}
	return circleAll;
}

void getInnerPaths(List *pathList, Group *group) {
	if(getLength(group->groups) > 0) {
		ListIterator iter = createIterator(group->groups);
		void *elem;
		while((elem = nextElement(&iter)) != NULL) {
			Group *g = (Group *)elem;
			getInnerPaths(pathList, g);
		}
	}

	ListIterator iter = createIterator(group->paths);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		Path *path = (Path *)elem;
		insertBack(pathList, (void *)path);
	}
}

// Function that returns a list of all paths in the image.
List* getPaths(SVGimage *img) {
	if(img == NULL) {
		return NULL;
	}

	List *pathAll = initializeList(&pathToString, &deletePath, &comparePaths);
	
	ListIterator iter = createIterator(img->paths);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		Path *path = (Path *)elem;
		insertBack(pathAll, (void *)path);
	}

	iter = createIterator(img->groups);
	while((elem = nextElement(&iter)) != NULL) {
		Group *group = (Group *)elem;
		getInnerPaths(pathAll, group);
	}
	return pathAll;
}

void getInnerGroups(List *groupList, Group *group) {
	if(getLength(group->groups) > 0) {
		ListIterator iter = createIterator(group->groups);
		void *elem;
		while((elem = nextElement(&iter)) != NULL) {
			Group *g = (Group *)elem;
			getInnerGroups(groupList, g);
		}
	}

	ListIterator iter = createIterator(group->groups);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		Group *g = (Group *)elem;
		insertBack(groupList, (void *)g);
	}
}

// Function that returns a list of all groups in the image.
List* getGroups(SVGimage *img) {
	if(img == NULL) {
		return NULL;
	}

	List *groupAll = initializeList(&groupToString, &deleteGroup, &compareGroups);

	ListIterator iter = createIterator(img->groups);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		Group *group = (Group *)elem;
		insertBack(groupAll, (void *)group);
	}

	iter = createIterator(img->groups);
	while((elem = nextElement(&iter)) != NULL) {
		Group *group = (Group *)elem;
		getInnerGroups(groupAll, group);
	}
	return groupAll;
}

/**
 * For the four "num..." functions below, you need to search the SVG image for components  that match the search
 * criterion.  You may wish to write some sort of a generic searcher fucntion that accepts an image, a predicate function,
 * and a dummy search record as arguments.  We will discuss such search functions in class
 * 
 * NOTE: For consistency, use the ceil() function to round the floats up to the nearest integer once you have computed
 * the number you need.  See A1 Module 2 for details.
 * 
 * @pre SVGimgage exists, is not null, and has not been freed.  The search criterion is valid
 * @post SVGimgage has not been modified in any way
 * @return an int indicating how many objects matching the criterion are contained in the image
 * @param obj - a pointer to an SVG struct
 * @param 2nd - the second param depends on the function.  See details below
 */

// Function that returns the number of all rectangles with the specified area
int numRectsWithArea(SVGimage *img, float area) {
	if(img == NULL) {
		return 0;
	}
	int num = 0;
	ListIterator iter = createIterator(getRects(img));
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		Rectangle *rect = (Rectangle *)elem;
		float a = rect->height * rect->width;
		if(ceil(area) == ceil(a)) {
			num++;
		}
	}
	return num;
}

// Function that returns the number of all circles with the specified area
int numCirclesWithArea(SVGimage *img, float area) {
	if(img == NULL) {
		return 0;
	}
	int num = 0;
	ListIterator iter = createIterator(getCircles(img));
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		Circle *circle = (Circle *)elem;
		float a = circle->r * circle->r * 3.1415926;
		if(ceil(area) == ceil(a)) {
			num++;
		}
	}
	return num;
}

// Function that returns the number of all paths with the specified data - i.e. Path.data field
int numPathsWithdata(SVGimage *img, char *data) {
	if(img == NULL) {
		return 0;
	}
	int num = 0;
	ListIterator iter = createIterator(getPaths(img));
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		Path *path = (Path *)elem;
		if(strcmp(data, path->data) == 0) {
			num++;
		}
	}
	return num;
}

// Function that returns the number of all groups with the specified length - see A1 Module 2 for details
int numGroupsWithLen(SVGimage *img, int len) {
	if(img == NULL) {
		return 0;
	}
	int num = 0;
	ListIterator iter = createIterator(getGroups(img));
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		Group *group = (Group *)elem;
		int size = getLength(group->rectangles) + getLength(group->circles) + getLength(group->paths) + getLength(group->groups);
		if(size == len) {
			num++;
		}
	}
	return num;
}

/**
 * Function that returns the total number of Attribute structs in the SVGimage - i.e. the number of Attributes
 * contained in all otherAttributes lists in the structs making up the SVGimage
 * @pre SVGimgage  exists, is not null, and has not been freed.
 * @post SVGimage has not been modified in any way
 * @return the total length of all attribute structs in the SVGimage
 * @param obj - a pointer to an SVG struct
 */
int numAttr(SVGimage *img) {
	if(img == NULL) {
		return 0;
	}
	int num = 0;
	num += getLength(img->otherAttributes);

	ListIterator iter;
	void *elem;
	if(getLength(getRects(img)) >= 0) {
		iter = createIterator(getRects(img));
		while((elem = nextElement(&iter)) != NULL) {
			Rectangle *rect = (Rectangle *)elem;
			num += getLength(rect->otherAttributes);
		}
	}

	if(getLength(getCircles(img)) >= 0) {
		iter = createIterator(getCircles(img));
		while ((elem = nextElement(&iter)) != NULL) {
			Circle *circle = (Circle *)elem;
			num += getLength(circle->otherAttributes);
		}
	}

	if(getLength(getPaths(img)) >= 0) {
		iter = createIterator(getPaths(img));
		while((elem = nextElement(&iter)) != NULL) {
			Path *path = (Path *)elem;
			num += getLength(path->otherAttributes);
		}
	}

	if(getLength(getGroups(img)) >= 0) {
		iter = createIterator(getGroups(img));
		while((elem = nextElement(&iter)) != NULL) {
			Group *group = (Group *)elem;
			num += getLength(group->otherAttributes);
		}
	}
	return num;
}

void deleteAttribute(void *data) {
	if(data == NULL) {
		return;
	}
	Attribute *attr = (Attribute *)data;
	if(attr->name != NULL) {
		free(attr->name);
	}
	if(attr->value != NULL) {
		free(attr->value);
	}
	free(attr);
}

char* attributeToString(void *data) {
	return NULL;
}

int compareAttributes(const void *first, const void *second) {
	return 0;
}

void deleteGroup(void *data) {
	if(data == NULL) {
		return;
	}
	Group *group = (Group *)data;
	freeList(group->rectangles);
	freeList(group->circles);
	freeList(group->paths);
	freeList(group->groups);
	freeList(group->otherAttributes);
	free(group);
}

char* groupToString(void *data) {
	return NULL;
}

int compareGroups(const void *first, const void *second) {
	return 0;
}

void deleteRectangle(void *data) {
	if(data == NULL) {
		return;
	}
	Rectangle *rect = (Rectangle *)data;
	freeList(rect->otherAttributes);
	free(rect);
}

char* rectangleToString(void *data) {
	return NULL;
}

int compareRectangles(const void *first, const void *second) {
	return 0;
}

void deleteCircle(void *data) {
	if(data == NULL) {
		return;
	}
	Circle *circle = (Circle *)data;
	freeList(circle->otherAttributes);
	free(circle);
}

char* circleToString(void *data) {
	return NULL;
}

int compareCircles(const void *first, const void *second) {
	return 0;
}

void deletePath(void *data) {
	if(data == NULL) {
		return;
	}
	Path *path = (Path *)data;
	freeList(path->otherAttributes);
	if(path->data != NULL) {
		free(path->data);
	}
	free(path);
}

char* pathToString(void *data) {
	return NULL;
}

int comparePaths(const void *first, const void *second) {
	return 0;
}

/****************************** A2 Stuff ******************************/

bool validateAttribute(List *otherAttributes) {
	ListIterator iter;
	void *elem;
	if(otherAttributes != NULL) {
		iter = createIterator(otherAttributes);
		while((elem = nextElement(&iter)) != NULL) {
			Attribute *a = (Attribute *)elem;
			if(a->name == NULL || a->value == NULL || strlen(a->name) == 0) {
				return false;
			}
			if(strcmp(a->name, "RandomStuff") == 0) {
				return false;
			}
		}
	} else {
		return false;
	}
	return true;
}

bool validateRects(List *rectangles) {
	ListIterator iter;
	void *elem;
	if(rectangles != NULL) {
		iter = createIterator(rectangles);
		while((elem = nextElement(&iter)) != NULL) {
			Rectangle *rect = (Rectangle *)elem;
			if(strlen(rect->units) > 2) {
				return false;
			}
			if(rect->x < 0 || rect->y < 0 || rect->height < 0 || rect->width < 0) {
				return false;
			}
			if(validateAttribute(rect->otherAttributes) == false) {
				return false;
			}
		}
	}
	return true;
}

bool validateCircles(List *circles) {
	ListIterator iter;
	void *elem;
	if(circles != NULL) {
		iter = createIterator(circles);
		while((elem = nextElement(&iter)) != NULL) {
			Circle *circle = (Circle *)elem;
			if(strlen(circle->units) > 2) {
				return false;
			}
			if(circle->cx < 0 || circle->cy < 0 || circle->r < 0) {
				return false;
			}
			if(validateAttribute(circle->otherAttributes) == false) {
				return false;
			}
		}
	}
	return true;
}

bool validatePaths(List *paths) {
	ListIterator iter;
	void *elem;
	if(paths != NULL) {
		iter = createIterator(paths);
		while((elem = nextElement(&iter)) != NULL) {
			Path *path = (Path *)elem;
			if(path->data == NULL || strlen(path->data) == 0) {
				return false;
			}
			if(validateAttribute(path->otherAttributes) == false) {
				return false;
			}
		}
	}
	return true;
}

bool validateGroups(List *groups) {
	ListIterator iter;
	void *elem;
	if(groups != NULL) {
		iter = createIterator(groups);
		while((elem = nextElement(&iter)) != NULL) {
			Group *group = (Group *)elem;
			if(validateAttribute(group->otherAttributes) == false) {
				return false;
			}
			if(validateRects(group->rectangles) == false) {
				return false;
			}
			if(validateCircles(group->circles) == false) {
				return false;
			}
			if(validatePaths(group->paths) == false) {
				return false;
			}
			if(validateGroups(group->groups) == false) {
				return false;
			}
		}
	}
	return true;
}

/**
 * Function to validating an existing a SVGimage object against a SVG schema file
 * @pre
 *  SVGimage object exists and is not NULL
 *  schema file name is not NULL/empty, and represents a valid schema file
 * @post SVGimage has not been modified in any way
 * @return the boolean aud indicating whether the SVGimage is valid
 * @param obj - a pointer to a GPXSVGimagedoc struct
 * @param obj - the name iof a schema file
 */
bool validateSVGimage(SVGimage *image, char *schemaFile) {
	if(image == NULL || schemaFile == NULL || strlen(schemaFile) == 0) {
		return false;
	}

	if(validateAttribute(image->otherAttributes) == false) {
		return false;
	}
	if(validateRects(image->rectangles) == false) {
		return false;
	}
	if(validateCircles(image->circles) == false) {
		return false;
	}
	if(validatePaths(image->paths) == false) {
		return false;
	}
	if(validateGroups(image->groups) == false) {
		return false;
	}
	return true;
}

/**
 * Function to create an SVG object based on the contents of an SVG file.
 * This function must validate the XML tree generated by libxml against a SVG schema file
 * before attempting to traverse the tree and create an SVGimage struct
 * @pre File name cannot be an empty string or NULL.
        File represented by this name must exist and must be readable.
        Schema file name is not NULL/empty, and represents a valid schema file
 * @post Either:
         A valid SVGimage has been created and its address was returned
		 or
		 An error occurred, or SVG file was invalid, and NULL was returned
 * @return the pinter to the new struct or NULL
 * @param fileName - a string containing the name of the SVG file
 */
SVGimage* createValidSVGimage(char *fileName, char *schemaFile) {
	if(fileName == NULL || schemaFile == NULL) {
		return NULL;
	}
	SVGimage *img = createSVGimage(fileName);
	if(img == NULL) {
		return NULL;
	}
	if(validateSVGimage(img, schemaFile) == false) {
		return NULL;
	}
	return img;
}

void writeAttribute(xmlNodePtr root_node, List *otherAttributes) {
	ListIterator iter;
	void *elem;
	if(otherAttributes != NULL) {
		iter = createIterator(otherAttributes);
		while((elem = nextElement(&iter)) != NULL) {
			Attribute *a = (Attribute *)elem;
			xmlNewProp(root_node, BAD_CAST a->name, BAD_CAST a->value);
		}
	}
}

void writeRects(xmlNodePtr root_node, List *rectangles) {
	ListIterator iter;
	void *elem;
	if(rectangles != NULL) {
		iter = createIterator(rectangles);
		while((elem = nextElement(&iter)) != NULL) {
			xmlNodePtr nodePtr;
			Rectangle *rect = (Rectangle *)elem;
			nodePtr = xmlNewChild(root_node, NULL, BAD_CAST "rect", NULL);
			char x[64];
			sprintf(x, "%f%s", rect->x, rect->units);
			xmlNewProp(nodePtr, BAD_CAST "x", BAD_CAST x);
			char y[64];
			sprintf(y, "%f%s", rect->y, rect->units);
			xmlNewProp(nodePtr, BAD_CAST "y", BAD_CAST y);
			char width[64];
			sprintf(width, "%f%s", rect->width, rect->units);
			xmlNewProp(nodePtr, BAD_CAST "width", BAD_CAST width);
			char height[64];
			sprintf(height, "%f%s", rect->height, rect->units);
			xmlNewProp(nodePtr, BAD_CAST "height", BAD_CAST height);
			writeAttribute(nodePtr, rect->otherAttributes);
		}
	}
}

void writeCircles(xmlNodePtr root_node, List *circles) {
	ListIterator iter;
	void *elem;
	if(circles != NULL) {
		iter = createIterator(circles);
		while((elem = nextElement(&iter)) != NULL) {
			xmlNodePtr nodePtr;
			Circle *circle = (Circle *)elem;
			nodePtr = xmlNewChild(root_node, NULL, BAD_CAST "circle", NULL);
			char cx[64];
			sprintf(cx, "%f%s", circle->cx, circle->units);
			xmlNewProp(nodePtr, BAD_CAST "cx", BAD_CAST cx);
			char cy[64];
			sprintf(cy, "%f%s", circle->cy, circle->units);
			xmlNewProp(nodePtr, BAD_CAST "cy", BAD_CAST cy);
			char r[64];
			sprintf(r, "%f%s", circle->r, circle->units);
			xmlNewProp(nodePtr, BAD_CAST "r", BAD_CAST r);
			writeAttribute(nodePtr, circle->otherAttributes);
		}
	}
}

void writePaths(xmlNodePtr root_node, List *paths) {
	ListIterator iter;
	void *elem;
	if(paths != NULL) {
		iter = createIterator(paths);
		while((elem = nextElement(&iter)) != NULL) {
			xmlNodePtr nodePtr;
			Path *path = (Path *)elem;
			nodePtr = xmlNewChild(root_node, NULL, BAD_CAST "path", NULL);
			xmlNewProp(nodePtr, BAD_CAST "d", BAD_CAST path->data);
			writeAttribute(nodePtr, path->otherAttributes);
		}
	}
}

void writeGroups(xmlNodePtr root_node, List *groups) {
	ListIterator iter;
	void *elem;
	if(groups != NULL) {
		iter = createIterator(groups);
		while((elem = nextElement(&iter)) != NULL) {
			xmlNodePtr nodePtr;
			Group *group = (Group *)elem;
			nodePtr = xmlNewChild(root_node, NULL, BAD_CAST "g", NULL);
			writeAttribute(nodePtr, group->otherAttributes);
			writeRects(nodePtr, group->rectangles);
			writeCircles(nodePtr, group->circles);
			writePaths(nodePtr, group->paths);
			writeGroups(nodePtr, group->groups);
		}
	}
}

/**
 * Function to writing a SVGimage into a file in SVG format.
 * @pre
 *   SVGimage object exists, is valid, and and is not NULL.
 *   fileName is not NULL, has the correct extension
 * @post SVGimage has not been modified in any way, and a file representing the
 *   SVGimage contents in SVG format has been created
 * @return a boolean value indicating success or failure of the write
 * @param
 *   doc - a pointer to a SVGimage struct
 *   fileName - the name of the output file
 */
bool writeSVGimage(SVGimage *image, char *fileName) {
	if(image == NULL || fileName == NULL) {
		return false;
	}
	if(validateSVGimage(image, fileName) == false) {
		return false;
	}

	xmlDocPtr doc = NULL;
	xmlNodePtr root_node = NULL;
	doc = xmlNewDoc(BAD_CAST "1.0");
	root_node = xmlNewNode(NULL, BAD_CAST "svg");
	xmlDocSetRootElement(doc, root_node);

	if(image->namespace != NULL && strlen(image->namespace) > 0) {
		root_node->ns = xmlNewNs(root_node, BAD_CAST image->namespace, NULL);
		xmlSetNs(root_node, root_node->ns);
	}
	xmlDocSetRootElement(doc, root_node);
	if(image->title != NULL && strlen(image->title) > 0) {
		xmlNewChild(root_node, NULL, BAD_CAST "title", BAD_CAST image->title);
	}
	if(image->description != NULL && strlen(image->description) > 0) {
		xmlNewChild(root_node, NULL, BAD_CAST "desc", BAD_CAST image->description);
	}

	writeAttribute(root_node, image->otherAttributes);
	writeRects(root_node, image->rectangles);
	writeCircles(root_node, image->circles);
	writePaths(root_node, image->paths);
	writeGroups(root_node, image->groups);

	xmlSaveFormatFileEnc(fileName, doc, "UTF-8", 1);
	xmlFreeDoc(doc);
	xmlCleanupParser();
	return true;
}

void setSvgimage(SVGimage *image, Attribute *newAttribute) {
	bool found = false;
	char *attrName = newAttribute->name;
	char *attrVal = newAttribute->value;

	ListIterator iter = createIterator(image->otherAttributes);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		Attribute *a = (Attribute *)elem;
		if(strcmp(a->name, attrName) == 0) {
			strcpy(a->value, attrVal);
			found = true;
		}
	}
	if(found == false) {
		Attribute *newAtrr = (Attribute *)malloc(sizeof(Attribute));
		newAtrr->name = (char *)malloc(sizeof(char) * 1024);
		newAtrr->value = (char *)malloc(sizeof(char) * 1024 * 32);
		strcpy(newAtrr->name, attrName);
		strcpy(newAtrr->value, attrVal);
		insertBack(image->otherAttributes, (void *)newAtrr);
	}
}

void setCircle(SVGimage *image, int elemIndex, Attribute *newAttribute) {
	if(elemIndex >= getLength(image->circles)) {
		return;
	}

	bool found = false;
	char *attrName = newAttribute->name;
	char *attrVal = newAttribute->value;

	ListIterator iter = createIterator(image->circles);
	void *elem;
	int i = 0;
	while((elem = nextElement(&iter)) != NULL && i < elemIndex) {
		i++;
	}
	Circle *circle = (Circle *)elem;
	if(strcmp(attrName, "cx") == 0) {
		circle->cx = atof(attrVal);
		found = true;
	} else if(strcmp(attrName, "cy") == 0) {
		circle->cy = atof(attrVal);
		found = true;
	} else if(strcmp(attrName, "r") == 0) {
		circle->r = atof(attrVal);
		found = true;
	} else {
		ListIterator iter2 = createIterator(circle->otherAttributes);
		void *elem2;
		while((elem2 = nextElement(&iter2)) != NULL) {
			Attribute *a = (Attribute *)elem2;
			if(strcmp(a->name, attrName) == 0) {
				strcpy(a->value, attrVal);
				found = true;
			}
		}
	}
	if(found == false) {
		Attribute *newAtrr = (Attribute *)malloc(sizeof(Attribute));
		newAtrr->name = (char *)malloc(sizeof(char) * 1024);
		newAtrr->value = (char *)malloc(sizeof(char) * 1024 * 32);
		strcpy(newAtrr->name, attrName);
		strcpy(newAtrr->value, attrVal);
		insertBack(circle->otherAttributes, (void *)newAtrr);
	}
}

void setRect(SVGimage *image, int elemIndex, Attribute *newAttribute) {
	if(elemIndex >= getLength(image->rectangles)) {
		return;
	}

	bool found = false;
	char *attrName = newAttribute->name;
	char *attrVal = newAttribute->value;
	
	ListIterator iter = createIterator(image->rectangles);
	void *elem;
	int i = 0;
	while((elem = nextElement(&iter)) != NULL && i < elemIndex) {
		i++;
	}
	Rectangle *rect = (Rectangle *)elem;
	if(strcmp(attrName, "x") == 0) {
		rect->x = atof(attrVal);
		found = true;
	} else if(strcmp(attrName, "y") == 0) {
		rect->y = atof(attrVal);
		found = true;
	} else if(strcmp(attrName, "width") == 0) {
		rect->width = atof(attrVal);
		found = true;
	} else if(strcmp(attrName, "height") == 0) {
		rect->height = atof(attrVal);
		found = true;
	} else {
		ListIterator iter2 = createIterator(rect->otherAttributes);
		void *elem2;
		while((elem2 = nextElement(&iter2)) != NULL) {
			Attribute *a = (Attribute *)elem2;
			if(strcmp(a->name, attrName) == 0) {
				strcpy(a->value, attrVal);
				found = true;
			}
		}
	}
	if(found == false) {
		Attribute *newAtrr = (Attribute *)malloc(sizeof(Attribute));
		newAtrr->name = (char *)malloc(sizeof(char) * 1024);
		newAtrr->value = (char *)malloc(sizeof(char) * 1024 * 32);
		strcpy(newAtrr->name, attrName);
		strcpy(newAtrr->value, attrVal);
		insertBack(rect->otherAttributes, (void *)newAtrr);
	}
}

void setPath(SVGimage *image, int elemIndex, Attribute *newAttribute) {
	if(elemIndex >= getLength(image->paths)) {
		return;
	}

	bool found = false;
	char *attrName = newAttribute->name;
	char *attrVal = newAttribute->value;

	ListIterator iter = createIterator(image->paths);
	void *elem;
	int i = 0;
	while((elem = nextElement(&iter)) != NULL && i < elemIndex) {
		i++;
	}
	Path *path = (Path *)elem;
	if(strcmp(attrName, "d") == 0) {
		strcpy(path->data, attrVal);
		found = true;
	} else {
		ListIterator iter2 = createIterator(path->otherAttributes);
		void *elem2;
		while((elem2 = nextElement(&iter2)) != NULL) {
			Attribute *a = (Attribute *)elem2;
			if(strcmp(a->name, attrName) == 0) {
				strcpy(a->value, attrVal);
				found = true;
			}
		}
	}
	if(found == false) {
		Attribute *newAtrr = (Attribute *)malloc(sizeof(Attribute));
		newAtrr->name = (char *)malloc(sizeof(char) * 1024);
		newAtrr->value = (char *)malloc(sizeof(char) * 1024 * 32);
		strcpy(newAtrr->name, attrName);
		strcpy(newAtrr->value, attrVal);
		insertBack(path->otherAttributes, (void *)newAtrr);
	}
}

void setGroup(SVGimage *image, int elemIndex, Attribute *newAttribute) {
	bool found = false;
	char *attrName = newAttribute->name;
	char *attrVal = newAttribute->value;

	if(elemIndex >= getLength(image->groups)) {
		return;
	}
	ListIterator iter = createIterator(image->groups);
	void *elem;
	int i = 0;
	while((elem = nextElement(&iter)) != NULL && i < elemIndex) {
		i++;
	}
	Group *group = (Group *)elem;
	ListIterator iter2 = createIterator(group->otherAttributes);
	void *elem2;
	while((elem2 = nextElement(&iter2)) != NULL) {
		Attribute *a = (Attribute *)elem2;
		if(strcmp(a->name, attrName) == 0) {
			strcpy(a->value, attrVal);
			found = true;
		}
	}
	if(found == false) {
		Attribute *newAtrr = (Attribute *)malloc(sizeof(Attribute));
		newAtrr->name = (char *)malloc(sizeof(char) * 1024);
		newAtrr->value = (char *)malloc(sizeof(char) * 1024 * 32);
		strcpy(newAtrr->name, attrName);
		strcpy(newAtrr->value, attrVal);
		insertBack(group->otherAttributes, (void *)newAtrr);
	}
}

/**
 * Function to setting an attribute in an SVGimage or component
 * @pre
 *   SVGimage object exists, is valid, and and is not NULL.
 *   newAttribute is not NULL
 * @post The appropriate attribute was set corectly
 * @return N/A
 * @param
 *   image - a pointer to an SVGimage struct
 *   elemType - enum value indicating elemtn to modify
 *   elemIndex - index of thje lement to modify
 *   newAttribute - struct containing name and value of the updated attribute
 */
void setAttribute(SVGimage *image, elementType elemType, int elemIndex, Attribute *newAttribute) {
	if(image == NULL) {
		return;
	}

	if(elemType == SVG_IMAGE) {
		setSvgimage(image, newAttribute);
	}
	else if(elemType == CIRC) {
		setCircle(image, elemIndex, newAttribute);
	}
	else if(elemType == RECT) {
		setRect(image, elemIndex, newAttribute);
	}
	else if(elemType == PATH) {
		setPath(image, elemIndex, newAttribute);
	}
	else if(elemType == GROUP) {
		setGroup(image, elemIndex, newAttribute);
	}
	free(newAttribute->name);
	free(newAttribute->value);
	free(newAttribute);
}

/**
 * Function to adding an element - Circle, Rectngle, or Path - to an SVGimage
 * @pre
 *   SVGimage object exists, is valid, and and is not NULL.
 *   newElement is not NULL
 * @post The appropriate element was added correctly
 * @return N/A
 * @param
 *   image - a pointer to an SVGimage struct
 *   elemType - enum value indicating elemtn to modify
 *   newElement - pointer to the element sgtruct (Circle, Rectngle, or Path)
 */
void addComponent(SVGimage *image, elementType type, void *newElement) {
	if(image == NULL || newElement == NULL) {
		return;
	}
	if(type == CIRC) {
		Circle *circle = (Circle *)newElement;
		insertBack(image->circles, circle);
	} else if(type == RECT) {
		Rectangle *rect = (Rectangle *)newElement;
		insertBack(image->rectangles, rect);
	} else if(type == PATH) {
		Path *path = (Path *)newElement;
		insertBack(image->paths, path);
	}
}

/**
 * Function to converting an Attribute into a JSON string
 * @pre Attribute is not NULL
 * @post Attribute has not been modified in any way
 * @return A string in JSON format
 * @param event - a pointer to an Attribute struct
 */
char* attrToJSON(const Attribute *a) {
	char *str = (char *)malloc(sizeof(char) * 1024 * 32);
	if(a == NULL) {
		strcpy(str, "{}");
		return str;
	}
	sprintf(str, "{\"name\":\"%s\",\"value\":\"%s\"}", a->name, a->value);
	return str;
}

/**
 * Function to converting a Circle into a JSON string
 * @pre Circle is not NULL
 * @post Circle has not been modified in any way
 * @return A string in JSON format
 * @param event - a pointer to a Circle struct
 */
char* circleToJSON(const Circle *c) {
	char *str = (char *)malloc(sizeof(char) * 1024);
	if(c == NULL) {
		strcpy(str, "{}");
		return str;
	}
	sprintf(str, "{\"cx\":%.2f,\"cy\":%.2f,\"r\":%.2f,\"numAttr\":%d,\"units\":\"%s\"}", c->cx, c->cy, c->r, getLength(c->otherAttributes), c->units);
	return str;
}

/**
 * Function to converting a Rectangle into a JSON string
 * @pre Rectangle is not NULL
 * @post Rectangle has not been modified in any way
 * @return A string in JSON format
 * @param event - a pointer to a Rectangle struct
 */
char* rectToJSON(const Rectangle *r) {
	char *str = (char *)malloc(sizeof(char) * 1024);
	if(r == NULL) {
		strcpy(str, "{}");
		return str;
	}
	sprintf(str, "{\"x\":%.2f,\"y\":%.2f,\"w\":%.2f,\"h\":%.2f,\"numAttr\":%d,\"units\":\"%s\"}", r->x, r->y, r->width, r->height, getLength(r->otherAttributes), r->units);
	return str;
}

/**
 * Function to converting a Path into a JSON string
 * @pre Path is not NULL
 * @post Path has not been modified in any way
 * @return A string in JSON format
 * @param event - a pointer to a Path struct
 */
char* pathToJSON(const Path *p) {
	char *str = (char *)malloc(sizeof(char) * 1024);
	char dVal[65];
	if(p == NULL) {
		strcpy(str, "{}");
		return str;
	}
	strncpy(dVal, p->data, 64);
	dVal[64] = '\0';
	sprintf(str, "{\"d\":\"%s\",\"numAttr\":%d}", dVal, getLength(p->otherAttributes));
	return str;
}

/**
 * Function to converting a Group into a JSON string
 * @pre Group is not NULL
 * @post Group has not been modified in any way
 * @return A string in JSON format
 * @param event - a pointer to a Group struct
 */
char* groupToJSON(const Group *g) {
	char *str = (char *)malloc(sizeof(char) * 1024);
	if(g == NULL) {
		strcpy(str, "{}");
		return str;
	}
	sprintf(str, "{\"children\":%d,\"numAttr\":%d}", getLength(g->rectangles) + getLength(g->circles) + getLength(g->paths) + getLength(g->groups), getLength(g->otherAttributes));
	return str;
}

/**
 * Function to converting a list of Attribute structs into a JSON string
 * @pre Attribute list is not NULL
 * @post Attribute list has not been modified in any way
 * @return A string in JSON format
 * @param event - a pointer to a List struct
 */
char* attrListToJSON(const List *list) {
	char *str = (char *)malloc(sizeof(char) * 1024 * 32);
	if(list == NULL) {
		strcpy(str, "[]");
		return str;
	}
	sprintf(str, "[");
	ListIterator iter = createIterator((List *)list);
	void *elem;
	bool flag = false;
	while((elem = nextElement(&iter)) != NULL) {
		flag = true;
		Attribute *a = (Attribute *)elem;
		char *s = attrToJSON(a);
		strcat(str, s);
		strcat(str, ",");
		free(s);
	}
	if(flag == true) {
		str[strlen(str)-1] = ']';
	} else {
		strcat(str, "]");
	}
	strcat(str, "\0");
	return str;
}

/**
 * Function to converting a list of Circle structs into a JSON string
 * @pre Circle list is not NULL
 * @post Circle list has not been modified in any way
 * @return A string in JSON format
 * @param event - a pointer to a List struct
 */
char* circListToJSON(const List *list) {
	char *str = (char *)malloc(sizeof(char) * 1024);
	if(list == NULL) {
		strcpy(str, "[]");
		return str;
	}
	sprintf(str, "[");
	ListIterator iter = createIterator((List *)list);
	void *elem;
	bool flag = false;
	while((elem = nextElement(&iter)) != NULL) {
		flag = true;
		Circle *circle = (Circle *)elem;
		char *s = circleToJSON(circle);
		strcat(str, s);
		strcat(str, ",");
		free(s);
	}
	if(flag == true) {
		str[strlen(str)-1] = ']';
	} else {
		strcat(str, "]");
	}
	strcat(str, "\0");
	return str;
}

/**
 * Function to converting a list of Rectangle structs into a JSON string
 * @pre Rectangle list is not NULL
 * @post Rectangle list has not been modified in any way
 * @return A string in JSON format
 * @param event - a pointer to a List struct
 */
char* rectListToJSON(const List *list) {
	char *str = (char *)malloc(sizeof(char) * 1024);
	if(list == NULL) {
		strcpy(str, "[]");
		return str;
	}
	sprintf(str, "[");
	ListIterator iter = createIterator((List *)list);
	void *elem;
	bool flag = false;
	while((elem = nextElement(&iter)) != NULL) {
		flag = true;
		Rectangle *rect = (Rectangle *)elem;
		char *s = rectToJSON(rect);
		strcat(str, s);
		strcat(str, ",");
		free(s);
	}
	if(flag == true) {
		str[strlen(str)-1] = ']';
	} else {
		strcat(str, "]");
	}
	strcat(str, "\0");
	return str;
}

/**
 * Function to converting a list of Path structs into a JSON string
 * @pre Path list is not NULL
 * @post Path list has not been modified in any way
 * @return A string in JSON format
 * @param event - a pointer to a List struct
 */
char* pathListToJSON(const List *list) {
	char *str = (char *)malloc(sizeof(char) * 1024);
	if(list == NULL) {
		strcpy(str, "[]");
		return str;
	}
	sprintf(str, "[");
	ListIterator iter = createIterator((List *)list);
	void *elem;
	bool flag = false;
	while((elem = nextElement(&iter)) != NULL) {
		flag = true;
		Path *path = (Path *)elem;
		char *s = pathToJSON(path);
		strcat(str, s);
		strcat(str, ",");
		free(s);
	}
	if(flag == true) {
		str[strlen(str)-1] = ']';
	} else {
		strcat(str, "]");
	}
	strcat(str, "\0");
	return str;
}

/**
 * Function to converting a list of Group structs into a JSON string
 * @pre Group list is not NULL
 * @post Group list has not been modified in any way
 * @return A string in JSON format
 * @param event - a pointer to a List struct
 */
char* groupListToJSON(const List *list) {
	char *str = (char *)malloc(sizeof(char) * 1024);
	if(list == NULL) {
		strcpy(str, "[]");
		return "[]";
	}
	sprintf(str, "[");
	ListIterator iter = createIterator((List *)list);
	void *elem;
	bool flag = false;
	while((elem = nextElement(&iter)) != NULL) {
		flag = true;
		Group *group = (Group *)elem;
		char *s = groupToJSON(group);
		strcat(str, s);
		strcat(str, ",");
		free(s);
	}
	if(flag == true) {
		str[strlen(str)-1] = ']';
	} else {
		strcat(str, "]");
	}
	strcat(str, "\0");
	return str;
}

void countInnerRects(Group *group, int *cnt) {
	if(getLength(group->groups) > 0) {
		ListIterator iter = createIterator(group->groups);
		void *elem;
		while((elem = nextElement(&iter)) != NULL) {
			Group *g = (Group *)elem;
			countInnerRects(g, cnt);
		}
	}

	ListIterator iter = createIterator(group->rectangles);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		(*cnt)++;
	}
}

int countRects(SVGimage *img) {
	int cnt = 0;

	ListIterator iter = createIterator(img->rectangles);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		cnt++;
	}

	iter = createIterator(img->groups);
	while((elem = nextElement(&iter)) != NULL) {
		Group *group = (Group *)elem;
		countInnerRects(group, &cnt);
	}
	return cnt;
}

void countInnerCircles(Group *group, int *cnt) {
	if(getLength(group->groups) > 0) {
		ListIterator iter = createIterator(group->groups);
		void *elem;
		while((elem = nextElement(&iter)) != NULL) {
			Group *g = (Group *)elem;
			countInnerCircles(g, cnt);
		}
	}

	ListIterator iter = createIterator(group->circles);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		(*cnt)++;
	}
}

int countCircles(SVGimage *img) {
	int cnt = 0;

	ListIterator iter = createIterator(img->circles);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		cnt++;
	}

	iter = createIterator(img->groups);
	while((elem = nextElement(&iter)) != NULL) {
		Group *group = (Group *)elem;
		countInnerCircles(group, &cnt);
	}
	return cnt;
}

void countInnerPaths(Group *group, int *cnt) {
	if(getLength(group->groups) > 0) {
		ListIterator iter = createIterator(group->groups);
		void *elem;
		while((elem = nextElement(&iter)) != NULL) {
			Group *g = (Group *)elem;
			countInnerPaths(g, cnt);
		}
	}

	ListIterator iter = createIterator(group->paths);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		(*cnt)++;
	}
}

int countPaths(SVGimage *img) {
	int cnt = 0;

	ListIterator iter = createIterator(img->paths);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		cnt++;
	}

	iter = createIterator(img->groups);
	while((elem = nextElement(&iter)) != NULL) {
		Group *group = (Group *)elem;
		countInnerPaths(group, &cnt);
	}
	return cnt;
}

void countInnerGroups(Group *group, int *cnt) {
	if(getLength(group->groups) > 0) {
		ListIterator iter = createIterator(group->groups);
		void *elem;
		while((elem = nextElement(&iter)) != NULL) {
			Group *g = (Group *)elem;
			countInnerGroups(g, cnt);
		}
	}

	ListIterator iter = createIterator(group->groups);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		(*cnt)++;
	}
}

int countGroups(SVGimage *img) {
	int cnt = 0;

	ListIterator iter = createIterator(img->groups);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		cnt++;
	}

	iter = createIterator(img->groups);
	while((elem = nextElement(&iter)) != NULL) {
		Group *group = (Group *)elem;
		countInnerGroups(group, &cnt);
	}
	return cnt;
}

/**
 * Function to converting an SVGimage into a JSON string
 * @pre SVGimage is not NULL
 * @post SVGimage has not been modified in any way
 * @return A string in JSON format
 * @param event - a pointer to an SVGimage struct
 */
char* SVGtoJSON(const SVGimage *image) {
	char *str = (char *)malloc(sizeof(char) * 1024);
	if(image == NULL) {
		strcpy(str, "{}");
		return str;
	}
	int numRects = countRects((SVGimage *)image);
	int numCircles = countCircles((SVGimage *)image);
	int numPaths = countPaths((SVGimage *)image);
	int numGroups = countGroups((SVGimage *)image);
	sprintf(str, "{\"numRect\":%d,\"numCirc\":%d,\"numPaths\":%d,\"numGroups\":%d}", numRects, numCircles, numPaths, numGroups);
	return str;
}

/* ******************************* Bonus A2 functions - optional for A2 *************************** */

/**
 * Function to converting a JSON string into an SVGimage struct
 * @pre JSON string is not NULL
 * @post String has not been modified in any way
 * @return A newly allocated and initialized SVGimage struct
 * @param str - a pointer to a string
 */
SVGimage* JSONtoSVG(const char *svgString) {
	if(svgString == NULL) {
		return NULL;
	}
	return NULL;
}

/**
 * Function to converting a JSON string into a Rectangle struct
 * @pre JSON string is not NULL
 * @post Rectangle has not been modified in any way
 * @return A newly allocated and initialized Rectangle struct
 * @param str - a pointer to a string
 */
Rectangle* JSONtoRect(const char *svgString) {
	if(svgString == NULL) {
		return NULL;
	}
	return NULL;
}

/**
 * Function to converting a JSON string into a Circle struct
 * @pre JSON string is not NULL
 * @post Circle has not been modified in any way
 * @return A newly allocated and initialized Circle struct
 * @param str - a pointer to a string
 */
Circle* JSONtoCircle(const char *svgString) {
	if(svgString == NULL) {
		return NULL;
	}
	return NULL;
}

int getFileSize(char *fileName) {
	FILE *fp = fopen(fileName, "r");
	if(fp == NULL) {
		return 0;
	}
	double size = 0.0;
	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	size /= 1000;
	size = (int)(size + 0.5);
	fclose(fp);
	return size;
}

char* titleToJSON(const SVGimage *image) {
	char *str = (char *)malloc(sizeof(char) * 1024);
	if(image == NULL) {
		strcpy(str, "{}");
		return str;
	}
	strcpy(str, image->title);
	return str;
}

char* descToJSON(const SVGimage *image) {
	char *str = (char *)malloc(sizeof(char) * 1024);
	if(image == NULL) {
		strcpy(str, "{}");
		return str;
	}
	strcpy(str, image->description);
	return str;
}

int countOuterRects(SVGimage *img) {
	int cnt = 0;
	ListIterator iter = createIterator(img->rectangles);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		cnt++;
	}
	return cnt;
}

int countOuterCircles(SVGimage *img) {
	int cnt = 0;
	ListIterator iter = createIterator(img->circles);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		cnt++;
	}
	return cnt;
}

int countOuterPaths(SVGimage *img) {
	int cnt = 0;
	ListIterator iter = createIterator(img->paths);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		cnt++;
	}
	return cnt;
}

int countOuterGroups(SVGimage *img) {
	int cnt = 0;
	ListIterator iter = createIterator(img->groups);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		cnt++;
	}
	return cnt;
}

List* getOuterRects(SVGimage *img) {
	if(img == NULL) {
		return NULL;
	}
	List *rects = initializeList(&rectangleToString, &deleteRectangle, &compareRectangles);
	ListIterator iter = createIterator(img->rectangles);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		Rectangle *r = (Rectangle *)elem;
		insertBack(rects, (void *)r);
	}
	return rects;
}

List* getOuterCircles(SVGimage *img) {
	if(img == NULL) {
		return NULL;
	}
	List *circles = initializeList(&circleToString, &deleteCircle, &compareCircles);
	ListIterator iter = createIterator(img->circles);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		Circle *c = (Circle *)elem;
		insertBack(circles, (void *)c);
	}
	return circles;
}

List* getOuterPaths(SVGimage *img) {
	if(img == NULL) {
		return NULL;
	}
	List *paths = initializeList(&pathToString, &deletePath, &comparePaths);
	ListIterator iter = createIterator(img->paths);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		Path *p = (Path *)elem;
		insertBack(paths, (void *)p);
	}
	return paths;
}

List* getOuterGroups(SVGimage *img) {
	if(img == NULL) {
		return NULL;
	}
	List *groups = initializeList(&groupToString, &deleteGroup, &compareGroups);
	ListIterator iter = createIterator(img->groups);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		Group *g = (Group *)elem;
		insertBack(groups, (void *)g);
	}
	return groups;
}

int countInnerElements(Group *group) {
	int cnt = 0;
	ListIterator iter = createIterator(group->rectangles);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		cnt++;
	}
	iter = createIterator(group->circles);
	while((elem = nextElement(&iter)) != NULL) {
		cnt++;
	}
	iter = createIterator(group->paths);
	while((elem = nextElement(&iter)) != NULL) {
		cnt++;
	}
	iter = createIterator(group->groups);
	while((elem = nextElement(&iter)) != NULL) {
		cnt++;
	}
	return cnt;
}

char* getRectsInfo(SVGimage *img) {
	List *list = getOuterRects(img);
	char *str = rectListToJSON(list);
	return str;
}

char* getCirclesInfo(SVGimage *img) {
	List *list = getOuterCircles(img);
	char *str = circListToJSON(list);
	return str;
}

char* getPathsInfo(SVGimage *img) {
	List *list = getOuterPaths(img);
	char *str = pathListToJSON(list);
	return str;
}

char* getGroupsInfo(SVGimage *img) {
	List *list = getOuterGroups(img);
	char *str = groupListToJSON(list);
	return str;
}

char* getRectAttrInfo(SVGimage *img) {
	List *rectList = getOuterRects(img);
	List *attrList = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
	ListIterator iter = createIterator(rectList);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		Rectangle *rect = (Rectangle *)elem;
		ListIterator iter2 = createIterator(rect->otherAttributes);
		void *elem2;
		while((elem2 = nextElement(&iter2)) != NULL) {
			Attribute *a = (Attribute *)elem2;
			insertBack(attrList, a);
		}
	}
	char *str = attrListToJSON(attrList);
	return str;
}

char* getCircAttrInfo(SVGimage *img) {
	List *circList = getOuterCircles(img);
	List *attrList = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
	ListIterator iter = createIterator(circList);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		Circle *circ = (Circle *)elem;
		ListIterator iter2 = createIterator(circ->otherAttributes);
		void *elem2;
		while((elem2 = nextElement(&iter2)) != NULL) {
			Attribute *a = (Attribute *)elem2;
			insertBack(attrList, a);
		}
	}
	char *str = attrListToJSON(attrList);
	return str;
}

char* getPathAttrInfo(SVGimage *img) {
	List *pathList = getOuterPaths(img);
	List *attrList = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
	ListIterator iter = createIterator(pathList);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		Path *path = (Path *)elem;
		ListIterator iter2 = createIterator(path->otherAttributes);
		void *elem2;
		while((elem2 = nextElement(&iter2)) != NULL) {
			Attribute *a = (Attribute *)elem2;
			insertBack(attrList, a);
		}
	}
	char *str = attrListToJSON(attrList);
	return str;
}

char* getGroupAttrInfo(SVGimage *img) {
	List *groupList = getOuterGroups(img);
	List *attrList = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
	ListIterator iter = createIterator(groupList);
	void *elem;
	while((elem = nextElement(&iter)) != NULL) {
		Group *group = (Group *)elem;
		ListIterator iter2 = createIterator(group->otherAttributes);
		void *elem2;
		while((elem2 = nextElement(&iter2)) != NULL) {
			Attribute *a = (Attribute *)elem2;
			insertBack(attrList, a);
		}
	}
	char *str = attrListToJSON(attrList);
	return str;
}

errorCode editTitle(char *fileName, char *text) {
	SVGimage *svg = createValidSVGimage(fileName, "schemaFile");
	if(svg == NULL) {
		return ERR;
	}
	strcpy(svg->title, text);
	writeSVGimage(svg, fileName);
	return SUCC;
}

errorCode editDesc(char *fileName, char *text) {
	SVGimage *svg = createValidSVGimage(fileName, "schemaFile");
	if(svg == NULL) {
		return ERR;
	}
	strcpy(svg->description, text);
	writeSVGimage(svg, fileName);
	return SUCC;
}

errorCode editAttribute(char *fileName, char *attrName, char *attrVal, char *type, int index) {
	SVGimage *svg = createValidSVGimage(fileName, "schemaFile");
	if(svg == NULL) {
		return ERR;
	}
	Attribute *a = (Attribute *)malloc(sizeof(Attribute));
	a->name = (char *)malloc(sizeof(char) * 32);
	a->value = (char *)malloc(sizeof(char) * 1024);
	strcpy(a->name, attrName);
	strcpy(a->value, attrVal);
	if(strcmp(type, "SVG_IMAGE") == 0) {
		if((strcmp(a->name, "width") == 0) || (strcmp(a->name, "height") == 0)) {
			strcat(a->value, "cm");
		}
		setAttribute(svg, SVG_IMAGE, index, a);
	} else if(strcmp(type, "RECT") == 0) {
		setAttribute(svg, RECT, index, a);
	} else if(strcmp(type, "CIRC") == 0) {
		setAttribute(svg, CIRC, index, a);
	} else if(strcmp(type, "PATH") == 0) {
		setAttribute(svg, PATH, index, a);
	} else if(strcmp(type, "GROUP") == 0) {
		setAttribute(svg, GROUP, index, a);
	}
	writeSVGimage(svg, fileName);
	return SUCC;
}

errorCode addRect(char *fileName, float x, float y, float width, float height, char *unit, char *attrName, char *attrVal) {
	SVGimage *svg = createValidSVGimage(fileName, "schemaFile");
	if(svg == NULL) {
		return ERR;
	}
	Rectangle *rect = (Rectangle *)malloc(sizeof(Rectangle));
	rect->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
	rect->x = x;
	rect->y = y;
	rect->width = width;
	rect->height = height;
	strcpy(rect->units, unit);
	if(attrName != NULL && strlen(attrName) > 0 && attrVal != NULL && strlen(attrVal) > 0) {
		Attribute *a = (Attribute *)malloc(sizeof(Attribute));
		a->name = (char *)malloc(sizeof(char) * 64);
		a->value = (char *)malloc(sizeof(char) * 64);
		strcpy(a->name, attrName);
		strcpy(a->value, attrVal);
		insertBack(rect->otherAttributes, a);
	}
	addComponent(svg, RECT, rect);
	writeSVGimage(svg, fileName);
	return SUCC;
}

errorCode addCirc(char *fileName, float cx, float cy, float r, char *unit, char *attrName, char *attrVal) {
	SVGimage *svg = createValidSVGimage(fileName, "schemaFile");
	if(svg == NULL) {
		return ERR;
	}
	Circle *circle = (Circle *)malloc(sizeof(Circle));
	circle->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
	circle->cx = cx;
	circle->cy = cy;
	circle->r = r;
	strcpy(circle->units, unit);
	if(attrName != NULL && strlen(attrName) > 0 && attrVal != NULL && strlen(attrVal) > 0) {
		Attribute *a = (Attribute *)malloc(sizeof(Attribute));
		a->name = (char *)malloc(sizeof(char) * 64);
		a->value = (char *)malloc(sizeof(char) * 64);
		strcpy(a->name, attrName);
		strcpy(a->value, attrVal);
		insertBack(circle->otherAttributes, a);
	}
	addComponent(svg, CIRC, circle);
	writeSVGimage(svg, fileName);
	return SUCC;
}

void createNewSVGImage(char *fileName, char *title, char *description) {
	SVGimage *svg = (SVGimage *)malloc(sizeof(SVGimage));
	svg->rectangles = initializeList(&rectangleToString, &deleteRectangle, &compareRectangles);
	svg->circles = initializeList(&circleToString, &deleteCircle, &compareCircles);
	svg->paths = initializeList(&pathToString, &deletePath, &comparePaths);
	svg->groups = initializeList(&groupToString, &deleteGroup, &compareGroups);
	svg->otherAttributes = initializeList(&attributeToString, &deleteAttribute, &compareAttributes);
	strcpy(svg->namespace, "http://www.w3.org/2000/svg");
	strcpy(svg->title, title);
	strcpy(svg->description, description);
	writeSVGimage(svg, fileName);
}

errorCode scaleShapes(char *fileName, char *shape, float factor) {
	SVGimage *svg = createValidSVGimage(fileName, "schemaFile");
	if(svg == NULL) {
		return ERR;
	}
	if(strcmp(shape, "rectangles") == 0) {
		List *rectList = getRects(svg);
		ListIterator iter = createIterator(rectList);
		void *elem;
		while((elem = nextElement(&iter)) != NULL) {
			Rectangle *rect = (Rectangle *)elem;
			rect->width *= factor;
			rect->height *= factor;
		}
	}
	else if(strcmp(shape, "circles") == 0) {
		List *circList = getCircles(svg);
		ListIterator iter = createIterator(circList);
		void *elem;
		while((elem = nextElement(&iter)) != NULL) {
			Circle *circ = (Circle *)elem;
			circ->r *= factor;
		}
	}
	writeSVGimage(svg, fileName);
	return SUCC;
}