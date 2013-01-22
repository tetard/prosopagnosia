uniform sampler2DRect src_tex_unit0;
uniform vec2 width;
uniform vec2 origin;

void main()
{
    vec2 dst = gl_TexCoord[0].xy - origin;
    vec2 off = dst * width * -0.05;
    vec4 color = gl_Color;
    color += texture2DRect(src_tex_unit0, (dst + off * 0.) + origin);
    color += texture2DRect(src_tex_unit0, (dst + off * 1.) + origin);
    color += texture2DRect(src_tex_unit0, (dst + off * 2.) + origin);
    color += texture2DRect(src_tex_unit0, (dst + off * 3.) + origin);
    color += texture2DRect(src_tex_unit0, (dst + off * 4.) + origin);
    color += texture2DRect(src_tex_unit0, (dst + off * 5.) + origin);
    color += texture2DRect(src_tex_unit0, (dst + off * 6.) + origin);
    color += texture2DRect(src_tex_unit0, (dst + off * 7.) + origin);
    color += texture2DRect(src_tex_unit0, (dst + off * 8.) + origin);
    color /= 9.0;
    gl_FragColor = color;
}
