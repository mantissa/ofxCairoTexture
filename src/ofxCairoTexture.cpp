
#include "ofxCairoTexture.h"
#include "ofGraphics.h"
#include "ofConstants.h"
#include "ofAppRunner.h"
#include "ofUtils.h"
#include "ofMesh.h"
#include "ofImage.h"

//-----------------------------------------------------------------------------------
ofxCairoTexture::ofxCairoTexture(){

	surface = NULL;
	cr = NULL;
	bBackgroundAuto = true;
}

//-----------------------------------------------------------------------------------
ofxCairoTexture::~ofxCairoTexture(){
	
	close();
}

//-----------------------------------------------------------------------------------
void ofxCairoTexture::setup(int w, int h){

	setup(false, ofRectangle(0, 0, w, h));
}

//-----------------------------------------------------------------------------------
void ofxCairoTexture::setup(bool b3D_, ofRectangle viewport_){

	if( viewport_.width == 0 || viewport_.height == 0 ){
		viewport_.set(0, 0, ofGetWidth(), ofGetHeight());
	}
	
	//printf("ofxCairoTexture::setup %f %f\n", viewport_.width, viewport_.height);
	
	surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, viewport_.width, viewport_.height);
	cr = cairo_create (surface);
	
	viewportRect = viewport_;
	viewport(viewportRect);
	
	b3D = b3D_;
}

//-----------------------------------------------------------------------------------
void ofxCairoTexture::save(string path, ofImageQualityType qualityLevel){

	tex.saveImage(path, qualityLevel);
}

//-----------------------------------------------------------------------------------
unsigned char * ofxCairoTexture::getPixels(){

	return cairo_image_surface_get_data (surface);
}

//-----------------------------------------------------------------------------------
void ofxCairoTexture::close(){
	
	//printf("closing\n");
	
	if(surface){
		cairo_surface_flush(surface);
		cairo_surface_finish(surface);
		cairo_surface_destroy(surface);
		surface = NULL;
	}
	if(cr){
		cairo_destroy(cr);
		cr = NULL;
	}
	
	tex.clear();
}

//-----------------------------------------------------------------------------------
void ofxCairoTexture::update(){
	
	if( surface == NULL ) {
		printf("Error: ofxCairoTexture not allocated\n");
		return;
	}
	
	int w = getWidth(); 
	int h = getHeight(); 
	unsigned char * pix = getPixels();
	
	tex.setFromPixels(pix, w, h, OF_IMAGE_COLOR_ALPHA, false);
}

//-----------------------------------------------------------------------------------
void ofxCairoTexture::draw(float x, float y, float z){

	tex.draw(x, y, z);
}


//-----------------------------------------------------------------------------------
void ct_helper_quadratic_to (cairo_t *cr, double x1, double y1, double x2, double y2){
	double x0, y0;
	cairo_get_current_point (cr, &x0, &y0);
	cairo_curve_to (cr,
					2.0 / 3.0 * x1 + 1.0 / 3.0 * x0,
					2.0 / 3.0 * y1 + 1.0 / 3.0 * y0,
					2.0 / 3.0 * x1 + 1.0 / 3.0 * x2,
					2.0 / 3.0 * y1 + 1.0 / 3.0 * y2,
					y1, y2);
}

//-----------------------------------------------------------------------------------
void ofxCairoTexture::draw(ofPath & shape){
	cairo_new_path(cr);
	vector<ofSubPath> & paths = shape.getSubPaths();
	for(int i=0;i<(int)paths.size();i++){
		draw(paths[i]);
	}
	
	cairo_fill_rule_t cairo_poly_mode;
	if(shape.getWindingMode()==OF_POLY_WINDING_ODD) cairo_poly_mode=CAIRO_FILL_RULE_EVEN_ODD;
	else cairo_poly_mode=CAIRO_FILL_RULE_WINDING;
	
	cairo_set_fill_rule(cr,cairo_poly_mode);
	
	
	ofColor prevColor;
	if(shape.getUseShapeColor()){
		//printf("using color!\n");
		prevColor = ofGetStyle().color;
	}
	
	if(shape.isFilled()){
		if(shape.getUseShapeColor()){
			ofColor c = shape.getFillColor() * ofGetStyle().color;
			c.a = shape.getFillColor().a/255. * ofGetStyle().color.a;
			//printf("color %i %i %i %i!\n", shape.getFillColor().r, shape.getFillColor().g, shape.getFillColor().b, shape.getFillColor().a);
			//printf("style %f %f %f %f!\n", (float)ofGetStyle().color.r/255.0, (float)ofGetStyle().color.g/255.0, (float)ofGetStyle().color.b/255.0, (float)ofGetStyle().color.a/255.0);
			//printf("alpha %f %f!\n", (float)c.a, ofGetStyle().color.a);
			cairo_set_source_rgba(cr, (float)c.r/255.0, (float)c.g/255.0, (float)c.b/255.0, (float)c.a/255.0);
		}
		
		if(shape.hasOutline()){
			cairo_fill_preserve( cr );
		}else{
			cairo_fill(cr);
		}
	}
	if(shape.hasOutline()){
		float lineWidth = ofGetStyle().lineWidth;
		if(shape.getUseShapeColor()){
			ofColor c = shape.getFillColor() * ofGetStyle().color;
			c.a = shape.getFillColor().a/255. * ofGetStyle().color.a;
			cairo_set_source_rgba(cr, (float)c.r/255.0, (float)c.g/255.0, (float)c.b/255.0, (float)c.a/255.0);
		}
		cairo_set_line_width( cr, shape.getStrokeWidth() );
		cairo_stroke( cr );
		cairo_set_line_width( cr, lineWidth );
	}
	
	if(shape.getUseShapeColor()){
		setColor(prevColor);
	}
	ofPopStyle();
}

//-----------------------------------------------------------------------------------
void ofxCairoTexture::draw(ofPolyline & poly){
	cairo_new_path(cr);
	for(int i=0;i<(int)poly.size();i++){
		cairo_line_to(cr,poly.getVertices()[i].x,poly.getVertices()[i].y);
	}
	if(poly.isClosed())
		cairo_close_path(cr);
	cairo_stroke( cr );
}

//-----------------------------------------------------------------------------------
void ofxCairoTexture::draw(vector<ofPoint> & vertexData, ofPrimitiveMode drawMode){
	if(vertexData.size()==0) return;
	pushMatrix();
	cairo_matrix_init_identity(getCairoMatrix());
	cairo_new_path(cr);
	//if(indices.getNumIndices()){
	
	int i = 1;
	ofVec3f v = transform(vertexData[0]);
	ofVec3f v2;
	cairo_move_to(cr,v.x,v.y);
	if(drawMode==OF_PRIMITIVE_TRIANGLE_STRIP){
		v = transform(vertexData[1]);
		cairo_line_to(cr,v.x,v.y);
		v = transform(vertexData[2]);
		cairo_line_to(cr,v.x,v.y);
		i=2;
	}
	for(; i<(int)vertexData.size(); i++){
		v = transform(vertexData[i]);
		switch(drawMode){
			case(OF_PRIMITIVE_TRIANGLES):
				if((i+1)%3==0){
					cairo_line_to(cr,v.x,v.y);
					v2 = transform(vertexData[i-2]);
					cairo_line_to(cr,v2.x,v2.y);
					cairo_move_to(cr,v.x,v.y);
				}else if((i+3)%3==0){
					cairo_move_to(cr,v.x,v.y);
				}else{
					cairo_line_to(cr,v.x,v.y);
				}
				
				break;
			case(OF_PRIMITIVE_TRIANGLE_STRIP):
				v2 = transform(vertexData[i-2]);
				cairo_line_to(cr,v.x,v.y);
				cairo_line_to(cr,v2.x,v2.y);
				cairo_move_to(cr,v.x,v.y);
				break;
			case(OF_PRIMITIVE_TRIANGLE_FAN):
				//triangles.addIndex((GLuint)0);
				//triangles.addIndex((GLuint)1);
				 //triangles.addIndex((GLuint)2);
				 //for(int i = 2; i < primitive.getNumVertices()-1;i++){
				 //triangles.addIndex((GLuint)0);
				 //triangles.addIndex((GLuint)i);
				 //triangles.addIndex((GLuint)i+1);
				 //}
				break;
			default:break;
		}
	}
	
	cairo_move_to(cr,vertexData[vertexData.size()-1].x,vertexData[vertexData.size()-1].y);
	cairo_stroke( cr );
	popMatrix();
}

//-----------------------------------------------------------------------------------
ofVec3f ofxCairoTexture::transform(ofVec3f vec){
	if(!b3D) return vec;
	vec = modelView.preMult(vec);
	vec = projection.preMult(vec);
	
	//vec.set(vec.x/vec.z*viewportRect.width*0.5-ofGetWidth()*0.5-viewportRect.x,vec.y/vec.z*viewportRect.height*0.5-ofGetHeight()*0.5-viewportRect.y);
	vec.set(vec.x/vec.z*ofGetWidth()*0.5,vec.y/vec.z*ofGetHeight()*0.5);
	return vec;
}

//-----------------------------------------------------------------------------------
void ofxCairoTexture::draw(ofMesh & primitive){
	if(primitive.getNumVertices()==0) return;
	pushMatrix();
	cairo_matrix_init_identity(getCairoMatrix());
	cairo_new_path(cr);
	//if(indices.getNumIndices()){
	
	int i = 1;
	ofVec3f v = transform(primitive.getVertex(primitive.getIndex(0)));
	ofVec3f v2;
	cairo_move_to(cr,v.x,v.y);
	if(primitive.getMode()==OF_PRIMITIVE_TRIANGLE_STRIP){
		v = transform(primitive.getVertex(primitive.getIndex(1)));
		cairo_line_to(cr,v.x,v.y);
		v = transform(primitive.getVertex(primitive.getIndex(2)));
		cairo_line_to(cr,v.x,v.y);
		i=2;
	}
	for(; i<primitive.getNumIndices(); i++){
		v = transform(primitive.getVertex(primitive.getIndex(i)));
		switch(primitive.getMode()){
			case(OF_PRIMITIVE_TRIANGLES):
				if((i+1)%3==0){
					cairo_line_to(cr,v.x,v.y);
					v2 = transform(primitive.getVertex(primitive.getIndex(i-2)));
					cairo_line_to(cr,v2.x,v2.y);
					cairo_move_to(cr,v.x,v.y);
				}else if((i+3)%3==0){
					cairo_move_to(cr,v.x,v.y);
				}else{
					cairo_line_to(cr,v.x,v.y);
				}
				
				break;
			case(OF_PRIMITIVE_TRIANGLE_STRIP):
				v2 = transform(primitive.getVertex(primitive.getIndex(i-2)));
				cairo_line_to(cr,v.x,v.y);
				cairo_line_to(cr,v2.x,v2.y);
				cairo_move_to(cr,v.x,v.y);
				break;
			case(OF_PRIMITIVE_TRIANGLE_FAN):
				//triangles.addIndex((GLuint)0);
				 //triangles.addIndex((GLuint)1);
				 //triangles.addIndex((GLuint)2);
				 //for(int i = 2; i < primitive.getNumVertices()-1;i++){
				 //triangles.addIndex((GLuint)0);
				 //triangles.addIndex((GLuint)i);
				 //triangles.addIndex((GLuint)i+1);
				 //}
				break;
			default:break;
		}
	}
	
	cairo_move_to(cr,primitive.getVertex(primitive.getIndex(primitive.getNumIndices()-1)).x,primitive.getVertex(primitive.getIndex(primitive.getNumIndices()-1)).y);
	
	if(ofGetStyle().lineWidth>0){
		
		cairo_stroke( cr );
	}
	popMatrix();
}

//-----------------------------------------------------------------------------------
void ofxCairoTexture::draw(ofMesh & vertexData, ofPolyRenderMode mode){
	draw(vertexData);
}

//-----------------------------------------------------------------------------------
void ofxCairoTexture::draw(ofSubPath & path){
	if(!surface || !cr) return;
	const vector<ofSubPath::Command> & commands = path.getCommands();
	cairo_new_sub_path(cr);
	for(int i=0; i<(int)commands.size(); i++){
		switch(commands[i].type){
			case ofSubPath::Command::lineTo:
				curvePoints.clear();
				cairo_line_to(cr,commands[i].to.x,commands[i].to.y);
				break;
				
				
			case ofSubPath::Command::curveTo:
				curvePoints.push_back(commands[i].to);
				
				//code adapted from ofxVectorGraphics to convert catmull rom to bezier
				if(curvePoints.size()==4){
					ofPoint p1=curvePoints[0];
					ofPoint p2=curvePoints[1];
					ofPoint p3=curvePoints[2];
					ofPoint p4=curvePoints[3];
					
					//SUPER WEIRD MAGIC CONSTANT = 1/6 (this works 100% can someone explain it?)
					ofPoint cp1 = p2 + ( p3 - p1 ) * (1.0/6);
					ofPoint cp2 = p3 + ( p2 - p4 ) * (1.0/6);
					
					cairo_curve_to( cr, cp1.x, cp1.y, cp2.x, cp2.y, p3.x, p3.y );
					curvePoints.pop_front();
				}
				break;
				
				
			case ofSubPath::Command::bezierTo:
				curvePoints.clear();
				cairo_curve_to(cr,commands[i].cp1.x,commands[i].cp1.y,commands[i].cp2.x,commands[i].cp2.y,commands[i].to.x,commands[i].to.y);
				break;
				
			case ofSubPath::Command::quadBezierTo:
				curvePoints.clear();
				cairo_curve_to(cr,commands[i].cp1.x,commands[i].cp1.y,commands[i].cp2.x,commands[i].cp2.y,commands[i].to.x,commands[i].to.y);
				break;
				
				
			case ofSubPath::Command::arc:
				curvePoints.clear();
				// elliptic arcs not directly supported in cairo, lets scale y
				if(commands[i].radiusX!=commands[i].radiusY){
					float ellipse_ratio = commands[i].radiusY/commands[i].radiusX;
					pushMatrix();
					translate(0,-commands[i].to.y*ellipse_ratio);
					scale(1,ellipse_ratio);
					translate(0,commands[i].to.y/ellipse_ratio);
					cairo_arc(cr,commands[i].to.x,commands[i].to.y,commands[i].radiusX,commands[i].angleBegin*DEG_TO_RAD,commands[i].angleEnd*DEG_TO_RAD);
					//cairo_set_matrix(cr,&stored_matrix);
					popMatrix();
				}else{
					cairo_arc(cr,commands[i].to.x,commands[i].to.y,commands[i].radiusX,commands[i].angleBegin*DEG_TO_RAD,commands[i].angleEnd*DEG_TO_RAD);
				}
				break;
		}
	}
	
	if(path.isClosed()){
		cairo_close_path(cr);
	}
	
	
}

//--------------------------------------------
void ofxCairoTexture::draw(ofImage & img, float x, float y, float z, float w, float h){
	ofPixelsRef pix = img.getPixelsRef();
	pushMatrix();
	translate(x,y,z);
	scale(w/pix.getWidth(),h/pix.getHeight());
	cairo_surface_t *image;
	int stride=0;
	int picsize = pix.getWidth()* pix.getHeight();
	unsigned char *imgPix = pix.getPixels();
	
	static vector<unsigned char> swapPixels;
	
	switch(pix.getImageType()){
		case OF_IMAGE_COLOR:
#ifdef TARGET_LITTLE_ENDIAN
			swapPixels.resize(picsize * 4);
			
			for(int p= 0; p<picsize; p++) {
				swapPixels[p*4] = imgPix[p*3 +2];
				swapPixels[p*4 +1] = imgPix[p*3 +1];
				swapPixels[p*4 +2] = imgPix[p*3];
			}
#else
			swapPixels.resize(picsize * 4);
			
			for(int p= 0; p<picsize; p++) {
				swapPixels[p*4] = imgPix[p*3];
				swapPixels[p*4 +1] = imgPix[p*3 +1];
				swapPixels[p*4 +2] = imgPix[p*3 +2];
			}
#endif
			stride = cairo_format_stride_for_width (CAIRO_FORMAT_RGB24, pix.getWidth());
			image = cairo_image_surface_create_for_data(&swapPixels[0], CAIRO_FORMAT_RGB24, pix.getWidth(), pix.getHeight(), stride);
			break;
		case OF_IMAGE_COLOR_ALPHA:
#ifdef TARGET_LITTLE_ENDIAN
			swapPixels.resize(picsize * 4);
			
			for(int p= 0; p<picsize; p++) {
				swapPixels[p*4] = imgPix[p*4+2];
				swapPixels[p*4 +1] = imgPix[p*4+1];
				swapPixels[p*4 +2] = imgPix[p*4];
				swapPixels[p*4 +3] = imgPix[p*4+3];
			}
			stride = cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, pix.getWidth());
			image = cairo_image_surface_create_for_data(&swapPixels[0], CAIRO_FORMAT_ARGB32, pix.getWidth(), pix.getHeight(), stride);
#else
			stride = cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, pix.getWidth());
			image = cairo_image_surface_create_for_data(pix.getPixels(), CAIRO_FORMAT_ARGB32, pix.getWidth(), pix.getHeight(), stride);
#endif
			break;
		case OF_IMAGE_GRAYSCALE:
			swapPixels.resize(picsize * 4);
			
			for(int p= 0; p<picsize; p++) {
				swapPixels[p*4] = imgPix[p];
				swapPixels[p*4 +1] = imgPix[p];
				swapPixels[p*4 +2] = imgPix[p];
			}
			stride = cairo_format_stride_for_width (CAIRO_FORMAT_RGB24, pix.getWidth());
			image = cairo_image_surface_create_for_data(&swapPixels[0], CAIRO_FORMAT_RGB24, pix.getWidth(), pix.getHeight(), stride);
			break;
		case OF_IMAGE_UNDEFINED:
			ofLog(OF_LOG_ERROR,"ofxCairoTexture: trying to render undefined type image");
			popMatrix();
			return;
			break;
	}
	cairo_set_source_surface (cr, image, 0,0);
	cairo_paint (cr);
	cairo_surface_flush(image);
	cairo_surface_destroy (image);
	popMatrix();
}

//--------------------------------------------
void ofxCairoTexture::draw(ofFloatImage & image, float x, float y, float z, float w, float h){
	ofImage tmp = image;
	draw(tmp,x,y,z,w,h);
}

//--------------------------------------------
void ofxCairoTexture::draw(ofShortImage & image, float x, float y, float z, float w, float h){
	ofImage tmp = image;
	draw(tmp,x,y,z,w,h);
}

//--------------------------------------------
void ofxCairoTexture::setRectMode(ofRectMode mode){
	rectMode = mode;
}

//--------------------------------------------
ofRectMode ofxCairoTexture::getRectMode(){
	return rectMode;
}

//--------------------------------------------
void ofxCairoTexture::setFillMode(ofFillFlag fill){
	bFilled = fill;
}

//--------------------------------------------
ofFillFlag ofxCairoTexture::getFillMode(){
	return bFilled;
}

//--------------------------------------------
void ofxCairoTexture::setLineWidth(float lineWidth){
	cairo_set_line_width( cr, lineWidth );
}

//--------------------------------------------
void ofxCairoTexture::setBlendMode(ofBlendMode blendMode){
	switch (blendMode){
		case OF_BLENDMODE_ALPHA:{
			cairo_set_operator(cr,CAIRO_OPERATOR_OVER);
			break;
		}
			
		case OF_BLENDMODE_ADD:{
			cairo_set_operator(cr,CAIRO_OPERATOR_ADD);
			break;
		}
			
		case OF_BLENDMODE_MULTIPLY:{
			cairo_set_operator(cr,CAIRO_OPERATOR_MULTIPLY);
			break;
		}
			
		case OF_BLENDMODE_SCREEN:{
			cairo_set_operator(cr,CAIRO_OPERATOR_SCREEN);
			break;
		}
			
		case OF_BLENDMODE_SUBTRACT:{
			cairo_set_operator(cr,CAIRO_OPERATOR_DIFFERENCE);
			break;
		}
			
			
		default:
			break;
	}
}

//--------------------------------------------
void ofxCairoTexture::setLineSmoothing(bool smooth){
	
}

// color options
//--------------------------------------------
void ofxCairoTexture::setColor(int r, int g, int b){
	setColor(r,g,b,255);
};

//--------------------------------------------
void ofxCairoTexture::setColor(int r, int g, int b, int a){
	cairo_set_source_rgba(cr, (float)r/255.0, (float)g/255.0, (float)b/255.0, (float)a/255.0);
};

//--------------------------------------------
void ofxCairoTexture::setColor(const ofColor & c){
	setColor(c.r,c.g,c.b,c.a);
};

//--------------------------------------------
void ofxCairoTexture::setColor(const ofColor & c, int _a){
	setColor(c.r,c.g,c.b,_a);
};

//--------------------------------------------
void ofxCairoTexture::setColor(int gray){
	setColor(gray,gray,gray,255);
};

//--------------------------------------------
void ofxCairoTexture::setHexColor( int hexColor ){
	int r = (hexColor >> 16) & 0xff;
	int g = (hexColor >> 8) & 0xff;
	int b = (hexColor >> 0) & 0xff;
	setColor(r,g,b);
};

//--------------------------------------------
// transformations
//our openGL wrappers
void ofxCairoTexture::pushMatrix(){
	if(!surface || !cr) return;
	cairo_get_matrix(cr,&tmpMatrix);
	matrixStack.push(tmpMatrix);
	
	if(!b3D) return;
	modelViewStack.push(modelView);
}

//-----------------------------------------------------------------------------------
void ofxCairoTexture::popMatrix(){
	if(!surface || !cr) return;
	cairo_set_matrix(cr,&matrixStack.top());
	matrixStack.pop();
	
	if(!b3D) return;
	modelView = modelViewStack.top();
	modelViewStack.pop();
}

//-----------------------------------------------------------------------------------
void ofxCairoTexture::translate(float x, float y, float z ){
	if(!surface || !cr) return;
	cairo_matrix_translate(getCairoMatrix(),x,y);
	setCairoMatrix();
	
	if(!b3D) return;
	modelView.glTranslate(ofVec3f(x,y,z));
	
}

//-----------------------------------------------------------------------------------
void ofxCairoTexture::translate(const ofPoint & p){
	translate(p.x,p.y,p.z);
}

//-----------------------------------------------------------------------------------
void ofxCairoTexture::scale(float xAmnt, float yAmnt, float zAmnt ){
	if(!surface || !cr) return;
	cairo_matrix_scale(getCairoMatrix(),xAmnt,yAmnt);
	setCairoMatrix();
	
	if(!b3D) return;
	modelView.glScale(xAmnt,yAmnt,zAmnt);
}

//-----------------------------------------------------------------------------------
void ofxCairoTexture::rotateZ(float degrees){
	if(!surface || !cr) return;
	cairo_matrix_rotate(getCairoMatrix(),degrees*DEG_TO_RAD);
	setCairoMatrix();
	
	if(!b3D) return;
	modelView.glRotate(180,0,0,1);
}

//-----------------------------------------------------------------------------------
void ofxCairoTexture::rotate(float degrees){
	rotateZ(degrees);
}

//-----------------------------------------------------------------------------------
void ofxCairoTexture::setupScreen(){
	if(!surface || !cr) return;
	
	setupScreenPerspective();	// assume defaults
}

//-----------------------------------------------------------------------------------
// screen coordinate things / default gl values
void ofxCairoTexture::pushView(){
	viewportStack.push(viewportRect);
}

//-----------------------------------------------------------------------------------
void ofxCairoTexture::popView(){
	viewportRect = viewportStack.top();
	viewportStack.pop();
};

//-----------------------------------------------------------------------------------
// setup matrices and viewport (upto you to push and pop view before and after)
// if width or height are 0, assume windows dimensions (ofGetWidth(), ofGetHeight())
// if nearDist or farDist are 0 assume defaults (calculated based on width / height)
void ofxCairoTexture::viewport(ofRectangle v){
	viewport(v.x,v.y,v.width,v.height);
}

//-----------------------------------------------------------------------------------
void ofxCairoTexture::viewport(float x, float y, float width, float height, bool invertY){
	if(width == 0) width = ofGetWindowWidth();
	if(height == 0) height = ofGetWindowHeight();
	
	if (invertY){
		y = ofGetWindowHeight() - (y + height);
	}
	
	
	viewportRect.set(x, y, width, height);
	
	
	cairo_reset_clip(cr);
	
	cairo_new_path(cr);
	
	cairo_move_to(cr,viewportRect.x,viewportRect.y);
	cairo_line_to(cr,viewportRect.x+viewportRect.width,viewportRect.y);
	cairo_line_to(cr,viewportRect.x+viewportRect.width,viewportRect.y+viewportRect.height);
	cairo_line_to(cr,viewportRect.x,viewportRect.y+viewportRect.height);
	/*
	cairo_clip(cr);
	 */
};

//-----------------------------------------------------------------------------------
void ofxCairoTexture::setupScreenPerspective(float width, float height, ofOrientation orientation, bool vFlip, float fov, float nearDist, float farDist){
	if(!b3D) return;
	if(width == 0) width = ofGetWidth();
	if(height == 0) height = ofGetHeight();
	if( orientation == OF_ORIENTATION_UNKNOWN ) orientation = ofGetOrientation();
	
	float viewW = ofGetViewportWidth();
	float viewH = ofGetViewportHeight();
	
	float eyeX = viewW / 2;
	float eyeY = viewH / 2;
	float halfFov = PI * fov / 360;
	float theTan = tanf(halfFov);
	float dist = eyeY / theTan;
	float aspect = (float) viewW / viewH;
	
	if(nearDist == 0) nearDist = dist / 10.0f;
	if(farDist == 0) farDist = dist * 10.0f;
	
	projection.makePerspectiveMatrix(fov,aspect,nearDist,farDist);
	modelView.makeLookAtViewMatrix(ofVec3f(eyeX,eyeY,dist),ofVec3f(eyeX,eyeY,0),ofVec3f(0,1,0));
	
	
	//note - theo checked this on iPhone and Desktop for both vFlip = false and true
	switch(orientation) {
		case OF_ORIENTATION_180:
			modelView.glRotate(-180,0,0,1);
			if(vFlip){
				modelView.glScale(-1,-1,1);
				modelView.glTranslate(width,0,0);
			}else{
				modelView.glTranslate(width,-height,0);
			}
			
			break;
			
		case OF_ORIENTATION_90_RIGHT:
			modelView.glRotate(-90,0,0,1);
			if(vFlip){
				modelView.glScale(1,1,1);
			}else{
				modelView.glScale(1,-1,1);
				modelView.glTranslate(-width,-height,0);
			}
			break;
			
		case OF_ORIENTATION_90_LEFT:
			modelView.glRotate(90,0,0,1);
			if(vFlip){
				modelView.glScale(1,1,1);
				modelView.glTranslate(0,-height,0);
			}else{
				
				modelView.glScale(1,-1,1);
				modelView.glTranslate(0,0,0);
			}
			break;
			
		case OF_ORIENTATION_DEFAULT:
		default:
			if(vFlip){
				modelView.glScale(-1,-1,1);
				modelView.glTranslate(-width,-height,0);
			}
			break;
	}
};

//-----------------------------------------------------------------------------------
void ofxCairoTexture::setupScreenOrtho(float width, float height, ofOrientation orientation, bool vFlip, float nearDist, float farDist){
	if(!b3D) return;
	if(width == 0) width = ofGetWidth();
	if(height == 0) height = ofGetHeight();
	if( orientation == OF_ORIENTATION_UNKNOWN ) orientation = ofGetOrientation();
	
	float viewW = ofGetViewportWidth();
	float viewH = ofGetViewportHeight();
	
	ofSetCoordHandedness(OF_RIGHT_HANDED);
	
	if(vFlip) {
		ofSetCoordHandedness(OF_LEFT_HANDED);
	}
	projection.makeOrthoMatrix(0, viewW, 0, viewH, nearDist, farDist);
	
	modelView.makeIdentityMatrix();
	
	//note - theo checked this on iPhone and Desktop for both vFlip = false and true
	switch(orientation) {
		case OF_ORIENTATION_180:
			modelView.glRotate(-180,0,0,1);
			if(vFlip){
				modelView.glScale(-1,-1,1);
				modelView.glTranslate(width,0,0);
			}else{
				modelView.glTranslate(width,-height,0);
			}
			
			break;
			
		case OF_ORIENTATION_90_RIGHT:
			modelView.glRotate(-90,0,0,1);
			if(vFlip){
				modelView.glScale(1,1,1);
			}else{
				modelView.glScale(1,-1,1);
				modelView.glTranslate(-width,-height,0);
			}
			break;
			
		case OF_ORIENTATION_90_LEFT:
			modelView.glRotate(90,0,0,1);
			if(vFlip){
				modelView.glScale(1,1,1);
				modelView.glTranslate(0,-height,0);
			}else{
				
				modelView.glScale(1,-1,1);
				modelView.glTranslate(0,0,0);
			}
			break;
			
		case OF_ORIENTATION_DEFAULT:
		default:
			if(vFlip){
				modelView.glScale(-1,-1,1);
				modelView.glTranslate(-width,-height,0);
			}
			break;
	}	
};

//-----------------------------------------------------------------------------------
ofRectangle ofxCairoTexture::getCurrentViewport(){
	return viewportRect;
};

//-----------------------------------------------------------------------------------
int ofxCairoTexture::getViewportWidth(){
	return viewportRect.width;
};

//-----------------------------------------------------------------------------------
int ofxCairoTexture::getViewportHeight(){
	return viewportRect.height;
};

//-----------------------------------------------------------------------------------
void ofxCairoTexture::setCoordHandedness(ofHandednessType handedness){
	
};

//-----------------------------------------------------------------------------------
ofHandednessType ofxCairoTexture::getCoordHandedness(){
	return OF_LEFT_HANDED;
};

//-----------------------------------------------------------------------------------
void ofxCairoTexture::setupGraphicDefaults(){
};

//-----------------------------------------------------------------------------------
void ofxCairoTexture::rotate(float degrees, float vecX, float vecY, float vecZ){
	if(!b3D) return;
	modelView.glRotate(degrees,vecX,vecY,vecZ);
}

//-----------------------------------------------------------------------------------
void ofxCairoTexture::rotateX(float degrees){
	if(!b3D) return;
	rotate(degrees,1,0,0);
}

//-----------------------------------------------------------------------------------
void ofxCairoTexture::rotateY(float degrees){
	if(!b3D) return;
	rotate(degrees,0,1,0);
}

//----------------------------------------------------------
void ofxCairoTexture::clear(float r, float g, float b, float a) {
	if(!surface || ! cr) return;
	cairo_set_source_rgba(cr,r/255., g/255., b/255., a/255.);
	cairo_paint(cr);
}

//----------------------------------------------------------
void ofxCairoTexture::clear(float brightness, float a) {
	clear(brightness, brightness, brightness, a);
}

//----------------------------------------------------------
void ofxCairoTexture::clearAlpha() {
}

//----------------------------------------------------------
void ofxCairoTexture::setBackgroundAuto(bool bAuto){
	bBackgroundAuto = bAuto;
}

//----------------------------------------------------------
bool ofxCairoTexture::bClearBg(){
	return bBackgroundAuto;
}

//----------------------------------------------------------
ofFloatColor & ofxCairoTexture::getBgColor(){
	return bgColor;
}

//----------------------------------------------------------
void ofxCairoTexture::background(const ofColor & c){
	bgColor = c;
	// if we are in auto mode, then clear with a bg call...
	if (bClearBg()){
		clear(c.r,c.g,c.b,c.a);
	}
}

//----------------------------------------------------------
void ofxCairoTexture::background(float brightness) {
	background(brightness);
}

//----------------------------------------------------------
void ofxCairoTexture::background(int hexColor, float _a){
	background ( (hexColor >> 16) & 0xff, (hexColor >> 8) & 0xff, (hexColor >> 0) & 0xff, _a);
}

//----------------------------------------------------------
void ofxCairoTexture::background(int r, int g, int b, int a){
	background(ofColor(r,g,b,a));
}


//----------------------------------------------------------
void ofxCairoTexture::drawLine(float x1, float y1, float z1, float x2, float y2, float z2){
	cairo_new_path(cr);
	cairo_move_to(cr,x1,y1);
	cairo_line_to(cr,x2,y2);
	
	cairo_stroke( cr );
}

//----------------------------------------------------------
void ofxCairoTexture::drawRectangle(float x, float y, float z, float w, float h){
	
	cairo_new_path(cr);
	
	if (ofGetStyle().rectMode == OF_RECTMODE_CORNER){
		cairo_move_to(cr,x,y);
		cairo_line_to(cr,x+w, y);
		cairo_line_to(cr,x+w, y+h);
		cairo_line_to(cr,x, y+h);
	}else{
		cairo_move_to(cr,x-w/2.0f, y-h/2.0f);
		cairo_line_to(cr,x+w/2.0f, y-h/2.0f);
		cairo_line_to(cr,x+w/2.0f, y+h/2.0f);
		cairo_line_to(cr,x-w/2.0f, y+h/2.0f);
	}
	
	cairo_close_path(cr);
	
	if(bFilled==OF_FILLED){
		cairo_fill( cr );
	}else{
		cairo_stroke( cr );
	}
}

//----------------------------------------------------------
void ofxCairoTexture::drawTriangle(float x1, float y1, float z1, float x2, float y2, float z2, float x3, float y3, float z3){
	cairo_new_path(cr);
	
	cairo_move_to(cr, x1, y1);
	cairo_line_to(cr, x2, y2);
	cairo_line_to(cr, x3, y3);
	
	
	cairo_close_path(cr);
	
	if(bFilled==OF_FILLED){
		cairo_fill( cr );
	}else{
		cairo_stroke( cr );
	}
}

//----------------------------------------------------------
void ofxCairoTexture::drawCircle(float x, float y, float z, float radius){
	cairo_new_path(cr);
	cairo_arc(cr, x,y,radius,0,2*PI);
	
	cairo_close_path(cr);
	
	if(bFilled==OF_FILLED){
		cairo_fill( cr );
	}else{
		cairo_stroke( cr );
	}
}

//----------------------------------------------------------
void ofxCairoTexture::drawEllipse(float x, float y, float z, float width, float height){
	cairo_new_path(cr);
	float ellipse_ratio = height/width;
	pushMatrix();
	translate(0,-y*ellipse_ratio);
	scale(1,ellipse_ratio);
	translate(0,y/ellipse_ratio);
	cairo_arc(cr,x,y,width*0.5,0,360);
	popMatrix();
	
	cairo_close_path(cr);
	
	
	if(bFilled==OF_FILLED){
		cairo_fill( cr );
	}else{
		cairo_stroke( cr );
	}
}

//-----------------------------------------------------------------------------------
void ofxCairoTexture::drawString(string text, float x, float y, float z, ofDrawBitmapMode mode){
	cairo_select_font_face (cr, "Mono", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, 10);
	vector<string> lines = ofSplitString(text, "\n");
	for(int i=0;i<(int)lines.size();i++){
		cairo_move_to (cr, x, y+i*14.3);
		cairo_show_text (cr, lines[i].c_str() );
	}
}

//-----------------------------------------------------------------------------------
cairo_t * ofxCairoTexture::getCairoContext(){
	return cr;
}

//-----------------------------------------------------------------------------------
cairo_surface_t * ofxCairoTexture::getCairoSurface(){
	return surface;
}

//-----------------------------------------------------------------------------------
cairo_matrix_t * ofxCairoTexture::getCairoMatrix(){
	cairo_get_matrix(cr,&tmpMatrix);
	return &tmpMatrix;
}

//-----------------------------------------------------------------------------------
void ofxCairoTexture::setCairoMatrix(){
	cairo_set_matrix(cr,&tmpMatrix);
}

//-----------------------------------------------------------------------------------
// size
int ofxCairoTexture::getWidth(){
	
	return getViewportWidth();
}

int ofxCairoTexture::getHeight(){
	
	return getViewportHeight();
}

