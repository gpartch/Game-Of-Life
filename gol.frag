uniform float dX;
uniform float dY;
uniform sampler2D img;

uniform float red;
uniform float green;
uniform float blue;



//  Get cell value
float cell(float dx,float dy)
{
   vec4 color = texture2D(img, gl_TexCoord[0].st + vec2(dx, dy));
   return (color.r > 0.0 || color.g > 0.0 || color.b > 0.0) ? 1.0 : 0.0;
}

// get color of cell
vec4 color()
{
   return texture2D(img, gl_TexCoord[0].st);
}

//  Evaluate cell
void main()
{
   //  Number of live neighbors
   float Nnb = cell(-dX,+dY) + cell(0.0,+dY) + cell(+dX,+dY)
              +cell(-dX,0.0) +               + cell(+dX,0.0)
              +cell(-dX,-dY) + cell(0.0,-dY) + cell(+dX,-dY);
   //  Decide if the cell is alive on the next cycle
   float live = (Nnb==3.0 || cell(0.0,0.0)==1.0 && Nnb==2.0) ? 1.0 : 0.0;
   //  Set the color to red if live, black if not

   // if alive and stays alive preserve color
   if(live && cell(0,0)) {gl_FragColor = color();}
   // if dead and becomes alive use new color
   else if(live) {gl_FragColor = vec4(red,green,blue,1.0);}
   // else stay black
   else {gl_FragColor = vec4(0.0,0.0,0.0,1.0);}
}
