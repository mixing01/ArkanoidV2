#version 330 core
out vec4 KolorFragmentu;

//tablica wyjœæ vertex shadera
in VS_OUT {
    vec3 positionOut;
    vec3 normalOut;
    vec2 texCoordsOut;
} fs_in;

uniform sampler2D diffuseTexture;

uniform vec3 lightPos;
uniform vec3 ballPos;
uniform vec3 viewPos;

void main()
{               
    vec3 normal = normalize(fs_in.normalOut);

    //kolor fragmentu
    vec3 color = texture(diffuseTexture, fs_in.texCoordsOut).rgb;

    //kolory œwiate³
    vec3 lightColor = vec3(0.5f);
    vec3 fireColor = vec3(0.89f,0.35f,0.13f);
    // otoczenie
    vec3 ka = 0.5 * lightColor;
    // rozproszenie œwiat³a
    vec3 kierunekDoSwiatla = normalize(lightPos - fs_in.positionOut);
    vec3 kd = max(dot(kierunekDoSwiatla, normal), 0.0) * lightColor;

    vec3 kierunekDoPilki = normalize(ballPos - fs_in.positionOut);
    vec3 kdb = max(dot(kierunekDoPilki, normal), 0.0) * fireColor;
    
    // odbicie
    vec3 kierunekDoObserwatora = normalize(viewPos - fs_in.positionOut);
    vec3 kierunekOdbitego = reflect(-kierunekDoSwiatla, normal);
    vec3 ks = pow(max(dot(kierunekDoObserwatora, kierunekOdbitego), 0.0), 5.0) * lightColor; 
    
    vec3 kierunekOdbitegoPilki = reflect(-kierunekDoPilki, normal);
    vec3 ksb = pow(max(dot(kierunekDoObserwatora, kierunekOdbitegoPilki), 0.0), 40.0) * fireColor;  
    // calculate shadow                        
    KolorFragmentu = vec4((ka + kd + kdb + ks + ksb) * color, 1.0);
}