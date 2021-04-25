#ifndef HELPER_H
#define HELPER_H

typedef enum CODE {
    ERR, SUCC
} errorCode;

char* extractContent(xmlNode *node);
Rectangle *extractRectangle(xmlNode *node);
Circle *extractCircle(xmlNode *node);
Path *extractPath(xmlNode *node);
Group *extractGroup(xmlNode *node, SVGimage *img);
bool validateAttribute(List *otherAttributes);
bool validateRects(List *rectangles);
bool validateCircles(List *circles);
bool validatePaths(List *paths);
bool validateGroups(List *groups);
void writeAttribute(xmlNodePtr root_node, List *otherAttributes);
void writeRects(xmlNodePtr root_node, List *rectangles);
void writeCircles(xmlNodePtr root_node, List *circles);
void writePaths(xmlNodePtr root_node, List *paths);
void writeGroups(xmlNodePtr root_node, List *groups);
void setSvgimage(SVGimage *image, Attribute *newAttribute);
void setCircle(SVGimage *image, int elemIndex, Attribute *newAttribute);
void setRect(SVGimage *image, int elemIndex, Attribute *newAttribute);
void setPath(SVGimage *image, int elemIndex, Attribute *newAttribute);
void setGroup(SVGimage *image, int elemIndex, Attribute *newAttribute);

int getFileSize(char *fileName);
char* titleToJSON(const SVGimage *image);
char* descToJSON(const SVGimage *image);
int countOuterRects(SVGimage *img);
int countOuterCircles(SVGimage *img);
int countOuterPaths(SVGimage *img);
int countOuterGroups(SVGimage *img);
List* getOuterRects(SVGimage *img);
List* getOuterCircles(SVGimage *img);
List* getOuterPaths(SVGimage *img);
List* getOuterGroups(SVGimage *img);
int countInnerElements(Group *group);
char* getRectsInfo(SVGimage *img);
char* getCirclesInfo(SVGimage *img);
char* getPathsInfo(SVGimage *img);
char* getGroupsInfo(SVGimage *img);
char* getRectAttrInfo(SVGimage *img);
char* getCircAttrInfo(SVGimage *img);
char* getPathAttrInfo(SVGimage *img);
char* getGroupAttrInfo(SVGimage *img);
errorCode editTitle(char *fileName, char *text);
errorCode editDesc(char *fileName, char *text);
errorCode editAttribute(char *fileName, char *attrName, char *attrVal, char *type, int index);
errorCode addRect(char *fileName, float x, float y, float width, float height, char *unit, char *attrName, char *attrVal);
errorCode addCirc(char *fileName, float cx, float cy, float r, char *unit, char *attrName, char *attrVal);
void createNewSVGImage(char *fileName, char *title, char *description);
errorCode scaleShapes(char *fileName, char *shape, float factor);

#endif
