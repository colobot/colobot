// math3d.cpp

#define STRICT
#define D3D_OVERLOADS

#include <math.h>
#include <stdio.h>
#include <d3d.h>

#include "struct.h"
#include "D3DEngine.h"
#include "D3DMath.h"
#include "D3DUtil.h"
#include "math3d.h"



// Retourne TRUE si 2 nombres sont presques égaux.

BOOL IsEqual(float a, float b)
{
	return Abs(a-b) < CHOUIA;
}


// Retourne la valeur minimale.

inline float Min(float a, float b)
{
	if ( a <= b )  return a;
	else           return b;
}

inline float Min(float a, float b, float c)
{
	return Min( Min(a,b), c );
}

inline float Min(float a, float b, float c, float d)
{
	return Min( Min(a,b), Min(c,d) );
}

inline float Min(float a, float b, float c, float d, float e)
{
	return Min( Min(a,b), Min(c,d), e );
}


// Retourne la valeur maximale.

inline float Max(float a, float b)
{
	if ( a >= b )  return a;
	else           return b;
}

inline float Max(float a, float b, float c)
{
	return Max( Max(a,b), c );
}

inline float Max(float a, float b, float c, float d)
{
	return Max( Max(a,b), Max(c,d) );
}

inline float Max(float a, float b, float c, float d, float e)
{
	return Max( Max(a,b), Max(c,d), e );
}


// Retourne la valeur normalisée (0..1).

inline float Norm(float a)
{
	if ( a < 0.0f )  return 0.0f;
	if ( a > 1.0f )  return 1.0f;
	return a;
}


// Retourne la valeur normalisée (-1..1).

inline float NormSign(float a)
{
	if ( a < -1.0f )  return -1.0f;
	if ( a >  1.0f )  return  1.0f;
	return a;
}


// Retourne la valeur absolue d'un nombre.

inline float Abs(float a)
{
	return (float)fabs(a);
}


// Permute deux entiers.

inline void Swap(int &a, int &b)
{
	int		c;

	c = a;
	a = b;
	b = c;
}

// Permute deux réels.

inline void Swap(float &a, float &b)
{
	float	c;

	c = a;
	a = b;
	b = c;
}

// Permute deux points.

inline void Swap(FPOINT &a, FPOINT &b)
{
	FPOINT	c;

	c = a;
	a = b;
	b = c;
}

// Retourne le modulo d'un nombre flottant.
//	Mod(8.1, 4) = 0.1
//	Mod(n, 1) = partie fractionnaire de n

inline float Mod(float a, float m)
{
	return a - ((int)(a/m))*m;
}

// Met une valeur sur la grille la plus proche.

float Grid(float value, float step)
{
	if ( value < 0.0f )
	{
		return (float)((int)((value-step/2.0f)/step)*step);
	}
	else
	{
		return (float)((int)((value+step/2.0f)/step)*step);
	}
}

D3DVECTOR Grid(D3DVECTOR pos, float step)
{
	return D3DVECTOR(Grid(pos.x,step), pos.y, Grid(pos.z,step));
}

// Retourne un angle normalisé, c'est-à-dire compris entre
// 0 et 2*PI.

inline float NormAngle(float angle)
{
	angle = Mod(angle, PI*2.0f);
	if ( angle < 0.0f )
	{
		return PI*2.0f + angle;
	}
	else
	{
		return angle;
	}
}

// Teste si un angle est compris entre 2 bornes.

BOOL TestAngle(float angle, float min, float max)
{
	angle = NormAngle(angle);
	min   = NormAngle(min);
	max   = NormAngle(max);

	if ( min > max )
	{
		return ( angle <= max || angle >= min );
	}
	else
	{
		return ( angle >= min && angle <= max );
	}
}


// Calcule l'angle permettant de tourner de l'angle a vers l'angle g.
// Un angle positif est anti-horaire (CCW).

float Direction(float a, float g)
{
	a = NormAngle(a);
	g = NormAngle(g);

	if ( a < g )
	{
		if ( a+PI*2.0f-g < g-a )  a += PI*2.0f;
	}
	else
	{
		if ( g+PI*2.0f-a < a-g )  g += PI*2.0f;
	}
	return (g-a);
}


// Fait tourner un point autour d'un centre.
// L'angle est exprimé en radians.
// Un angle positif est anti-horaire (CCW).

FPOINT RotatePoint(FPOINT center, float angle, FPOINT p)
{
	FPOINT	a, b;

	a.x = p.x-center.x;
	a.y = p.y-center.y;

	b.x = a.x*cosf(angle) - a.y*sinf(angle);
	b.y = a.x*sinf(angle) + a.y*cosf(angle);

	b.x += center.x;
	b.y += center.y;
	return b;
}

// Fait tourner un point autour de l'origine.
// L'angle est exprimé en radians.
// Un angle positif est anti-horaire (CCW).

FPOINT RotatePoint(float angle, FPOINT p)
{
	FPOINT	a;

	a.x = p.x*cosf(angle) - p.y*sinf(angle);
	a.y = p.x*sinf(angle) + p.y*cosf(angle);

	return a;
}

// Fait tourner un vecteur (dist;0).
// L'angle est exprimé en radians.
// Un angle positif est anti-horaire (CCW).

FPOINT RotatePoint(float angle, float dist)
{
	FPOINT	a;

	a.x = dist*cosf(angle);
	a.y = dist*sinf(angle);

	return a;
}

// Calcule l'angle d'un triangle rectangle.
// L'angle est anti-horaire (CCW), compris entre 0 et 2*PI.
// Pour obtenir un angle horaire (CW), il suffit de passer -y.
//
//      ^
//      |
//    y o----o
//      |  / |
//      |/)a |
//  ----o----o-->
//      |    x 
//      |

float RotateAngle(float x, float y)
{
#if 1
	if ( x == 0.0f && y == 0.0f )  return 0.0f;

	if ( x >= 0.0f )
	{
		if ( y >= 0.0f )
		{
			if ( x > y )  return           atanf(y/x);
			else          return PI*0.5f - atanf(x/y);
		}
		else
		{
			if ( x > -y )  return PI*2.0f + atanf(y/x);
			else           return PI*1.5f - atanf(x/y);
		}
	}
	else
	{
		if ( y >= 0.0f )
		{
			if ( -x > y )  return PI*1.0f + atanf(y/x);
			else           return PI*0.5f - atanf(x/y);
		}
		else
		{
			if ( -x > -y )  return PI*1.0f + atanf(y/x);
			else            return PI*1.5f - atanf(x/y);
		}
	}
#else
	float	angle;

	if ( x == 0.0f )
	{
		if ( y > 0.0f )
		{
			return 90.0f*PI/180.0f;
		}
		else
		{
			return 270.0f*PI/180.0f;
		}
	}
	else
	{
		angle = atanf(y/x);
		if ( x < 0.0f )
		{
			angle += PI;
		}
		return angle;
	}
#endif
}

// Calcule l'angle entre deux points et un centre.
// L'angle est exprimé en radians.
// Un angle positif est anti-horaire (CCW).

float RotateAngle(FPOINT center, FPOINT p1, FPOINT p2)
{
	float	a1, a2, a;

	if ( p1.x == center.x &&
		 p1.y == center.y )  return 0;

	if ( p2.x == center.x &&
		 p2.y == center.y )  return 0;

	a1 = asinf((p1.y-center.y)/Length(p1,center));
	a2 = asinf((p2.y-center.y)/Length(p2,center));

	if ( p1.x < center.x )  a1 = PI-a1;
	if ( p2.x < center.x )  a2 = PI-a2;

	a = a2-a1;
	if ( a < 0 )  a += PI*2;
	return a;
}

// Retourne py placé sur la droite ab.

float MidPoint(FPOINT a, FPOINT b, float px)
{
	if ( Abs(a.x-b.x) < CHOUIA )
	{
		if ( a.y < b.y )  return  BEAUCOUP;
		else              return -BEAUCOUP;
	}
	return (b.y-a.y)*(px-a.x)/(b.x-a.x)+a.y;
}

// Fait progresser un point le long d'une droite.

D3DVECTOR LineProgress(const D3DVECTOR &start, const D3DVECTOR &goal, float progress)
{
	return start+(goal-start)*progress;
}

// Vérifie si un point est dans un triangle.

BOOL IsInsideTriangle(FPOINT a, FPOINT b, FPOINT c, FPOINT p)
{
	float	n, m;

	if ( p.x < a.x && p.x < b.x && p.x < c.x )  return FALSE;
	if ( p.x > a.x && p.x > b.x && p.x > c.x )  return FALSE;
	if ( p.y < a.y && p.y < b.y && p.y < c.y )  return FALSE;
	if ( p.y > a.y && p.y > b.y && p.y > c.y )  return FALSE;

	if ( a.x > b.x )  Swap(a,b);
	if ( a.x > c.x )  Swap(a,c);
	if ( c.x < a.x )  Swap(c,a);
	if ( c.x < b.x )  Swap(c,b);

	n = MidPoint(a, b, p.x);
	m = MidPoint(a, c, p.x);
	if ( (n>p.y||p.y>m) && (n<p.y||p.y<m) )  return FALSE;

	n = MidPoint(c, b, p.x);
	m = MidPoint(c, a, p.x);
	if ( (n>p.y||p.y>m) && (n<p.y||p.y<m) )  return FALSE;

	return TRUE;
}

// Calcule la déformation d'un point sur une droite.

FPOINT Deforme(FPOINT a, FPOINT b, FPOINT c, FPOINT d, FPOINT p)
{
	FPOINT	pp;
	float	Lab, Lap, Lcd, Lcp;

	Lab = Length(a,b);
	Lap = Length(a,p);
	Lcd = Length(c,d);
	Lcp = Lap*Lcd/Lab;

	if ( Lcd == 0.0f )
	{
		pp = c;
	}
	else
	{
		pp.x = c.x+(d.x-c.x)*Lcp/Lcd;
		pp.y = c.y+(d.y-c.y)*Lcp/Lcd;
	}

	return pp;
}

// Calcule la déformation du point p dans le triangle t1,
// déformé en un nouveau triangle t2.

BOOL Deforme(FPOINT *t1, FPOINT *t2, FPOINT &p)
{
	FPOINT	i12, i20, ii12, ii20;

	if ( !Intersect(t1[0],p, t1[1],t1[2], i12) )  return FALSE;
	if ( !Intersect(t1[1],p, t1[2],t1[0], i20) )  return FALSE;

	ii12 = Deforme(t1[1],t1[2], t2[1],t2[2], i12);
	ii20 = Deforme(t1[2],t1[0], t2[2],t2[0], i20);

	if ( !Intersect(t2[0],ii12, t2[1],ii20, p) )  return FALSE;

	return TRUE;
}

// Calcule le point d'intersection entre deux droites ab et cd.
// Utilise l'algorithme de Gauss-Jordan utilisé pour le calcul
// matriciel. Les calculs ont été spécialisés au cas simple de
// l'intersection de segments pour des questions de rapidité.
//
//		Q=BX-AX : T=BY-AY
//		R=CX-DX : U=CY-DY   matrice  [ Q R | S ]
//		S=CX-AX : V=CY-AY            [ T U | V ]
//
// Cette matrice représente les coefficients de l'équation
// vectorielle suivante :
//		AB*a + CD*b = AC
//
// La coordonnée du point "P" s'obtient par :
//		P = OA + a*AB
//
// ou encore :
//		P = OC + b*CD
//
// Traite les cas particuliers des segments confondus ou parallèles.

BOOL Intersect(const FPOINT &a, const FPOINT &b,
			   const FPOINT &c, const FPOINT &d,
			   FPOINT &p)
{
	float	q,r,s,t,u,v;

	q = b.x-a.x;
	r = c.x-d.x;
	s = c.x-a.x;
	t = b.y-a.y;
	u = c.y-d.y;
	v = c.y-a.y;

	if ( q == 0.0f )  // ab vertical ?
	{
		if ( r == 0.0f )  // cd vertical ?
		{
			return FALSE;
		}
		else
		{
			p.x = ((d.x-c.x)*s/r)+c.x;
			p.y = ((d.y-c.y)*s/r)+c.y;
			return TRUE;
		}
	}

	if ( t != 0.0f )  // ab pas horizontal ?
	{
		u = u-(t*r)/q;
		v = v-(t*s)/q;
	}

	if ( u == 0.0f )
	{
		return FALSE;
	}

	p.x = ((d.x-c.x)*v/u)+c.x;
	p.y = ((d.y-c.y)*v/u)+c.y;
	return TRUE;
}

// Teste si un point est dans un rectangle p1-p2.

BOOL IsBBox(const FPOINT &p1, const FPOINT &p2, const FPOINT &p)
{
	if ( p1.x < p2.x )
	{
		if ( p.x < p1.x || p.x > p2.x )  return FALSE;
	}
	else
	{
		if ( p.x < p2.x || p.x > p1.x )  return FALSE;
	}
	if ( p1.y < p2.y )
	{
		if ( p.y < p1.y || p.y > p2.y )  return FALSE;
	}
	else
	{
		if ( p.y < p2.y || p.y > p1.y )  return FALSE;
	}
	return TRUE;
}

// Intersection de deux droites, avec test si le point
// est bien sur les deux segments ab et cd.

BOOL IntersectSegment(const FPOINT &a, const FPOINT &b,
					  const FPOINT &c, const FPOINT &d,
					  FPOINT &p)
{
	if ( !Intersect(a,b, c,d, p) )  return FALSE;
	if ( !IsBBox(a,b, p) )  return FALSE;
	if ( !IsBBox(c,d, p) )  return FALSE;
	return TRUE;
}

// Calcule l'intersection "i" de la droite "de" avec le plan "abc".

BOOL Intersect(D3DVECTOR a, D3DVECTOR b, D3DVECTOR c,
			   D3DVECTOR d, D3DVECTOR e, D3DVECTOR &i)
{
	float	d1, d2;

	d1 = (d.x-a.x)*((b.y-a.y)*(c.z-a.z)-(c.y-a.y)*(b.z-a.z)) -
		 (d.y-a.y)*((b.x-a.x)*(c.z-a.z)-(c.x-a.x)*(b.z-a.z)) +
		 (d.z-a.z)*((b.x-a.x)*(c.y-a.y)-(c.x-a.x)*(b.y-a.y));

	d2 = (d.x-e.x)*((b.y-a.y)*(c.z-a.z)-(c.y-a.y)*(b.z-a.z)) -
		 (d.y-e.y)*((b.x-a.x)*(c.z-a.z)-(c.x-a.x)*(b.z-a.z)) +
		 (d.z-e.z)*((b.x-a.x)*(c.y-a.y)-(c.x-a.x)*(b.y-a.y));

	if ( d2 == 0 )  return FALSE;

	i.x = d.x + d1/d2*(e.x-d.x);
	i.y = d.y + d1/d2*(e.y-d.y);
	i.z = d.z + d1/d2*(e.z-d.z);
	return TRUE;
}

// Calcule l'intersection de la droite passant par p(x,z) parallèle
// à l'axe y, avec le plan abc. Retourne p.y.

BOOL IntersectY(D3DVECTOR a, D3DVECTOR b, D3DVECTOR c, D3DVECTOR &p)
{
#if 0
	D3DVECTOR	d,e,i;

	d.x = p.x;
	d.y = 0.0f;
	d.z = p.z;
	e.x = p.x;
	e.y = 1.0f;
	e.z = p.z;
	if ( !Intersect(a,b,c,d,e,i) )  return FALSE;
	p.y = i.y;
	return TRUE;
#else
	float	d, d1, d2;

	d  = (b.x-a.x)*(c.z-a.z) - (c.x-a.x)*(b.z-a.z);
	d1 = (p.x-a.x)*(c.z-a.z) - (c.x-a.x)*(p.z-a.z);
	d2 = (b.x-a.x)*(p.z-a.z) - (p.x-a.x)*(b.z-a.z);

	if ( d == 0.0f )  return FALSE;

	p.y = a.y + d1/d*(b.y-a.y) + d2/d*(c.y-a.y);
	return TRUE;
#endif
}


// Avance de "dist" le long du segment p1-p2.

D3DVECTOR SegmentDist(const D3DVECTOR &p1, const D3DVECTOR &p2, float dist)
{
	return p1+Normalize(p2-p1)*dist;
}


// Teste si un point p est "dans" un segment p1-p2.
// Pour un segment vertical partant de bas en haut, être
// dedans, c'est être à gauche.
// Une suite de segments CCW permet de décrire une forme.
// Si un point est "IsInside" pour tous les segments, il est
// à l'intérieur la forme (si elle est convexe).

BOOL IsInside(FPOINT p1, FPOINT p2, FPOINT p)
{
	p2.x -= p1.x;
	p2.y -= p1.y;

	p.x -= p1.x;
	p.y -= p1.y;

	if ( p2.y > 0.0f )
	{
		if ( p2.x > 0.0f )
		{
			if ( p2.x > p2.y )  // octan 1 ?
			{
				return p.y >= p2.y/p2.x*p.x;
			}
			else	// octan 2 ?
			{
				return p.x <= p2.x/p2.y*p.y;
			}
		}
		else
		{
			if ( -p2.x < p2.y )  // octan 3 ?
			{
				return p.x <= p2.x/p2.y*p.y;
			}
			else	// octan 4 ?
			{
				return p.y <= p2.y/p2.x*p.x;
			}
		}
	}
	else
	{
		if ( p2.x < 0.0f )
		{
			if ( -p2.x > -p2.y )  // octan 5 ?
			{
				return p.y <= p2.y/p2.x*p.x;
			}
			else	// octan 6 ?
			{
				return p.x >= p2.x/p2.y*p.y;
			}
		}
		else
		{
			if ( p2.x < -p2.y )  // octan 7 ?
			{
				return p.x >= p2.x/p2.y*p.y;
			}
			else	// octan 8 ?
			{
				return p.y >= p2.y/p2.x*p.x;
			}
		}
	}
}


// Fait tourner un point autour d'un centre dans le plan.
// L'angle est exprimé en radians.
// Un angle positif est anti-horaire (CCW).

void RotatePoint(float cx, float cy, float angle, float &px, float &py)
{
	float	ax, ay;

	px -= cx;
	py -= cy;

	ax = px*cosf(angle) - py*sinf(angle);
	ay = px*sinf(angle) + py*cosf(angle);

	px = cx+ax;
	py = cy+ay;
}

// Fait tourner un point autour d'un centre dans l'espace.
// L'angles sont exprimés en radians.
// Un angle positif est anti-horaire (CCW).

void RotatePoint(D3DVECTOR center, float angleH, float angleV, D3DVECTOR &p)
{
	D3DVECTOR	a, b;

	p.x -= center.x;
	p.y -= center.y;
	p.z -= center.z;

	b.x = p.x*cosf(angleH) - p.z*sinf(angleH);
	b.y = p.z*sinf(angleV) + p.y*cosf(angleV);
	b.z = p.x*sinf(angleH) + p.z*cosf(angleH);

	p.x = center.x+b.x;
	p.y = center.y+b.y;
	p.z = center.z+b.z;
}

// Fait tourner un point autour d'un centre dans l'espace.
// L'angles sont exprimés en radians.
// Un angle positif est anti-horaire (CCW).

void RotatePoint2(D3DVECTOR center, float angleH, float angleV, D3DVECTOR &p)
{
	D3DVECTOR	a, b;

	p.x -= center.x;
	p.y -= center.y;
	p.z -= center.z;

	a.x = p.x*cosf(angleH) - p.z*sinf(angleH);
	a.y = p.y;
	a.z = p.x*sinf(angleH) + p.z*cosf(angleH);

	b.x = a.x;
	b.y = a.z*sinf(angleV) + a.y*cosf(angleV);
	b.z = a.z*cosf(angleV) - a.y*sinf(angleV);

	p.x = center.x+b.x;
	p.y = center.y+b.y;
	p.z = center.z+b.z;
}

// Calcul le point de vue permettant de regarder un centre selon deux
// angles et à une certaine distance.

D3DVECTOR RotateView(D3DVECTOR center, float angleH, float angleV, float dist)
{
	D3DMATRIX	mat1, mat2, mat;
	D3DVECTOR	eye;

	D3DUtil_SetRotateZMatrix(mat1, -angleV);
	D3DUtil_SetRotateYMatrix(mat2, -angleH);
	D3DMath_MatrixMultiply(mat, mat1, mat2);

	eye.x = 0.0f+dist;
	eye.y = 0.0f;
	eye.z = 0.0f;
	eye = Transform(mat, eye);

	return eye+center;
}

// Calcule le point d'arrivée.

D3DVECTOR LookatPoint( D3DVECTOR eye, float angleH, float angleV, float length )
{
	D3DVECTOR	lookat;

	lookat = eye;
	lookat.z += length;

//?	RotatePoint(eye.x, eye.z, angleH, lookat.x, lookat.z);
//?	RotatePoint(eye.z, eye.y, angleV, lookat.z, lookat.y);
	RotatePoint(eye, angleH, angleV, lookat);

	return lookat;
}


// Retourne la distance entre deux points.

inline float Length(FPOINT a, FPOINT b)
{
	return sqrtf( (a.x-b.x)*(a.x-b.x) +
				  (a.y-b.y)*(a.y-b.y) );
}

// Retourne l'hypothénuse d'un triangle rectangle.

inline float Length(float x, float y)
{
	return sqrtf( (x*x) + (y*y) );
}

// Retourne la longueur d'un vecteur.

inline float Length(const D3DVECTOR &u)
{
	return sqrtf( (u.x*u.x) + (u.y*u.y) + (u.z*u.z) );
}

// Retourne la distance entre deux points.

inline float Length(const D3DVECTOR &a, const D3DVECTOR &b)
{
	return sqrtf( (a.x-b.x)*(a.x-b.x) +
				  (a.y-b.y)*(a.y-b.y) +
				  (a.z-b.z)*(a.z-b.z) );
}

// Retourne la distance "à plat" entre deux points.

inline float Length2d(const D3DVECTOR &a, const D3DVECTOR &b)
{
	return sqrtf( (a.x-b.x)*(a.x-b.x) +
				  (a.z-b.z)*(a.z-b.z) );
}


// Retourne l'angle formé par deux vecteurs.

float Angle( D3DVECTOR u, D3DVECTOR v )
{
#if 0
	return acosf( Abs(u.x*v.x + u.y*v.y + u.z*v.z) / (Length(u)*Length(v)) );
#endif
#if 0
	float	d;
	d = (u.y*v.z-u.z*v.y) + (u.z*v.x-u.x*v.z) + (u.x*v.y-u.y*v.x);
	return asinf( Abs(d) / (Length(u)*Length(v)) );
#endif
#if 0
	return asinf( Length(Cross(u,v)) / (Length(u)*Length(v)) );
#endif
#if 1
	float	len, a, b;

	len = Length(u)*Length(v);
	a = acosf( (u.x*v.x + u.y*v.y + u.z*v.z) / len );
	b = asinf( Length(Cross(u,v)) / len );
	return a;
#endif
}

// Retourne le produit vectoriel de deux vecteurs.

inline D3DVECTOR Cross( D3DVECTOR u, D3DVECTOR v )
{
	return D3DVECTOR( u.y*v.z - u.z*v.y,
					  u.z*v.x - u.x*v.z,
					  u.x*v.y - u.y*v.x );
}

// Retourne le vecteur normal d'une face triangulaire.

D3DVECTOR ComputeNormal( D3DVECTOR p1, D3DVECTOR p2, D3DVECTOR p3 )
{
	D3DVECTOR	u, v;

	u = D3DVECTOR( p3.x-p1.x, p3.y-p1.y, p3.z-p1.z );
	v = D3DVECTOR( p2.x-p1.x, p2.y-p1.y, p2.z-p1.z );

	return Normalize(Cross(u, v));
}


// Transforme un point selon une matrice, exactement de la
// même façon que Direct3D.

D3DVECTOR Transform(const D3DMATRIX &m, D3DVECTOR p)
{
	D3DVECTOR	pp;

	pp.x = p.x*m._11 + p.y*m._21 + p.z*m._31 + m._41;
	pp.y = p.x*m._12 + p.y*m._22 + p.z*m._32 + m._42;
	pp.z = p.x*m._13 + p.y*m._23 + p.z*m._33 + m._43;

	return pp;
}


// Calcule la projection d'un point P sur une droite AB.

FPOINT Projection(const FPOINT &a, const FPOINT &b, const FPOINT &p)
{
	FPOINT	pp;
	float	k;

	k  = (b.x-a.x)*(p.x-a.x) + (b.y-a.y)*(p.y-a.y);
	k /= (b.x-a.x)*(b.x-a.x) + (b.y-a.y)*(b.y-a.y);

	pp.x = a.x + k*(b.x-a.x);
	pp.y = a.y + k*(b.y-a.y);
	return pp;
}

// Calcule la projection d'un point P sur une droite AB.

D3DVECTOR Projection(const D3DVECTOR &a, const D3DVECTOR &b, const D3DVECTOR &p)
{
	float		k;

	k  = (b.x-a.x)*(p.x-a.x) + (b.y-a.y)*(p.y-a.y) + (b.z-a.z)*(p.z-a.z);
	k /= (b.x-a.x)*(b.x-a.x) + (b.y-a.y)*(b.y-a.y) + (b.z-a.z)*(b.z-a.z);

	return a + k*(b-a);
}

// Plaque la texture dans le plan xz.

void MappingObject(D3DVERTEX2* pVertices, int nb, float scale)
{
	int		i;

	for ( i=0 ; i<nb ; i++ )
	{
		pVertices[i].tu = pVertices[i].x*scale;
		pVertices[i].tv = pVertices[i].z*scale;
	}
}

// Adoucit les normales.

void SmoothObject(D3DVERTEX2* pVertices, int nb)
{
	char*		bDone;
	int			index[100];
	int			i, j, rank;
	D3DVECTOR	sum;

	bDone = (char*)malloc(nb*sizeof(char));
	ZeroMemory(bDone, nb*sizeof(char));

	for ( i=0 ; i<nb ; i++ )
	{
		bDone[i] = TRUE;
		rank = 0;
		index[rank++] = i;

		for ( j=0 ; j<nb ; j++ )
		{
			if ( bDone[j] )  continue;
			if ( pVertices[j].x == pVertices[i].x &&
				 pVertices[j].y == pVertices[i].y &&
				 pVertices[j].z == pVertices[i].z )
			{
				bDone[j] = TRUE;
				index[rank++] = j;
				if ( rank >= 100 )  break;
			}
		}

		sum.x = 0;
		sum.y = 0;
		sum.z = 0;
		for ( j=0 ; j<rank ; j++ )
		{
			sum.x += pVertices[index[j]].nx;
			sum.y += pVertices[index[j]].ny;
			sum.z += pVertices[index[j]].nz;
		}
		sum = Normalize(sum);

		for ( j=0 ; j<rank ; j++ )
		{
			pVertices[index[j]].nx = sum.x;
			pVertices[index[j]].ny = sum.y;
			pVertices[index[j]].nz = sum.z;
		}
	}

	free(bDone);
}



// Calcule les paramètres a et b du segment passant par
// les points p1 et p2, sachant que :
//		f(x) = ax+b
// Retourne FALSE si la droite est verticale.

BOOL LineFunction(FPOINT p1, FPOINT p2, float &a, float &b)
{
	if ( D3DMath_IsZero(p1.x-p2.x) )
	{
		a = g_HUGE;  // pente infinie !
		b = p2.x;
		return FALSE;
	}

	a = (p2.y-p1.y)/(p2.x-p1.x);
	b = p2.y - p2.x*a;
	return TRUE;
}


// Calcule la distance entre un plan ABC et un point P.

float DistancePlanPoint(const D3DVECTOR &a, const D3DVECTOR &b,
						const D3DVECTOR &c, const D3DVECTOR &p)
{
	D3DVECTOR	n;
	float		aa,bb,cc,dd;

	n = ComputeNormal(a,b,c);

	aa = n.x;
	bb = n.y;
	cc = n.z;
	dd = -(n.x*a.x + n.y*a.y + n.z*a.z);

	return Abs(aa*p.x + bb*p.y + cc*p.z + dd);
}

// Vérifie si deux plans définis par 3 points font partie
// du même plan.

BOOL IsSamePlane(D3DVECTOR *plan1, D3DVECTOR *plan2)
{
	D3DVECTOR	n1, n2;
	float		dist;

	n1 = ComputeNormal(plan1[0], plan1[1], plan1[2]);
	n2 = ComputeNormal(plan2[0], plan2[1], plan2[2]);

	if ( Abs(n1.x-n2.x) > 0.1f ||
		 Abs(n1.y-n2.y) > 0.1f ||
		 Abs(n1.z-n2.z) > 0.1f )  return FALSE;

	dist = DistancePlanPoint(plan1[0], plan1[1], plan1[2], plan2[0]);
	if ( dist > 0.1f )  return FALSE;

	return TRUE;
}


// Calcule la matrice permettant de faire 3 rotations
// dans l'ordre X, Z et Y.
// >>>>>> A OPTIMISER !!!

void MatRotateXZY(D3DMATRIX &mat, D3DVECTOR angle)
{
	D3DMATRIX	temp;

	D3DUtil_SetRotateXMatrix(temp, angle.x);
	D3DUtil_SetRotateZMatrix(mat, angle.z);
	D3DMath_MatrixMultiply(mat, mat, temp);
	D3DUtil_SetRotateYMatrix(temp, angle.y);
	D3DMath_MatrixMultiply(mat, mat, temp);  // X-Z-Y
}

// Calcule la matrice permettant de faire 3 rotations
// dans l'ordre Z, X et Y.
// >>>>>> A OPTIMISER !!!

void MatRotateZXY(D3DMATRIX &mat, D3DVECTOR angle)
{
	D3DMATRIX	temp;

	D3DUtil_SetRotateZMatrix(temp, angle.z);
	D3DUtil_SetRotateXMatrix(mat, angle.x);
	D3DMath_MatrixMultiply(mat, mat, temp);
	D3DUtil_SetRotateYMatrix(temp, angle.y);
	D3DMath_MatrixMultiply(mat, mat, temp);  // Z-X-Y
}


// Retourne une valeur aléatoire comprise entre 0 et 1.

float Rand()
{
	return (float)rand()/RAND_MAX;
}

// Retourne une valeur pseudo-aléatoire comprise entre 0 et 1,
// toujours identique pour un même nombre de départ.

float Rand(float init)
{
	static const int A = 48271;
	static const int M = 2147483647;
	static const int Q = M / A;
	static const int R = M % A;

	int		state, tmp;

	state = (int)init;

	if ( state < 0 )  state += M;
	if ( state == 0 )  state = 1;
 
	tmp = A * ( state % Q ) - R * ( state / Q );
	if ( tmp >= 0 )
	{
		state = tmp;
	}
	else
	{
		state = tmp + M;
	}

	return (float) state / M;
}


// Générateur pseudo-aléatoire. Une valeur donnée en entrée
// produit toujours le même nombre "aléatoire".

void PseudoRandom(D3DVERTEX2 &p, float rv, float rh)
{
#if 1
	D3DVECTOR	i;

	i.x = p.x;
	i.y = p.y;
	i.z = p.z;

	p.x = Grid(p.x, 0.25f);
	p.y = Grid(p.y, 0.25f);
	p.z = Grid(p.z, 0.25f);

	if ( p.x <= -70.0f || p.x >= 70.0f ||
		 p.z <= -70.0f || p.z >= 70.0f )  rv = 2.0f;

	rv = rv*Norm((i.y+16.0f)/8.0f);
	p.y += (Rand(i.x*5841234.68f+i.z*5494311.57f)-0.5f)*2.0f*rv;
	if ( p.y < i.y )  p.y = i.y;

	p.x += (Rand(i.x*8002353.91f+i.z*4862025.33f)-0.5f)*2.0f*rh;
	p.z += (Rand(i.x*7432954.03f+i.z*6954024.82f)-0.5f)*2.0f*rh;
#else
	D3DVECTOR	i;

	i.x = p.x;
	i.y = p.y;
	i.z = p.z;

	p.x = Grid(p.x, 0.25f);
	p.y = Grid(p.y, 0.25f);
	p.z = Grid(p.z, 0.25f);

	if ( p.x <= -70.0f || p.x >= 70.0f ||
		 p.z <= -70.0f || p.z >= 70.0f )  rv = 2.0f;

	rv = rv*Norm((i.y+16.0f)/8.0f);
//?	p.y += sinf(i.x*0.89f)*rv;
//?	p.y += sinf(i.x*1.13f)*rv;
	p.y += sinf(i.x*2.31f)*rv;
	p.y += sinf(i.x*3.77f)*rv;
//?	p.y += sinf(i.z*0.77f)*rv;
//?	p.y += sinf(i.z*1.48f)*rv;
	p.y += sinf(i.z*2.69f)*rv;
	p.y += sinf(i.z*3.53f)*rv;
	if ( p.y < i.y )  p.y = i.y;

	p.x += sinf(i.x*0.43f)*rh;
	p.x += sinf(i.x*0.61f)*rh;
	p.x += sinf(i.z*2.48f)*rh;
	p.x += sinf(i.z*3.77f)*rh;

	p.z += sinf(i.x*0.43f)*rh;
	p.z += sinf(i.x*0.61f)*rh;
	p.z += sinf(i.z*2.48f)*rh;
	p.z += sinf(i.z*3.77f)*rh;
#endif
}


// Gestion de la zone neutre d'un joystick.

//  in:   -1            0            1
//       --|-------|----o----|-------|-->
//                      <---->
//                       dead
//  out:  -1       0         0       1

float Neutral(float value, float dead)
{
	if ( Abs(value) <= dead )
	{
		return 0.0f;
	}
	else
	{
		if ( value > 0.0f )  return (value-dead)/(1.0f-dead);
		else                 return (value+dead)/(1.0f-dead);
	}
}


// Calcule une valeur (radians) proportionnelle comprise
// entre a et b (degrés).

inline float Prop(int a, int b, float p)
{
	float	aa, bb;

	aa = (float)a*PI/180.0f;
	bb = (float)b*PI/180.0f;

	return aa+p*(bb-aa);
}

// Fait progresser linéairement une valeur souhaitée à partir
// de sa valeur actuelle.

float Linear(float actual, float hope, float time)
{
	float	futur;

	if ( actual == hope )  return hope;

	if ( hope > actual )
	{
		futur = actual+time;
		if ( futur > hope )  futur = hope;
	}
	if ( hope < actual )
	{
		futur = actual-time;
		if ( futur < hope )  futur = hope;
	}

	return futur;
}

// Fait progresser mollement une valeur souhaitée à partir de
// sa valeur actuelle. Plus le temps est grand et plus la
// progression est rapide.

float Smooth(float actual, float hope, float time)
{
	float	futur;

	if ( actual == hope )  return hope;

	futur = actual + (hope-actual)*time;

	if ( hope > actual )
	{
		if ( futur > hope )  futur = hope;
	}
	if ( hope < actual )
	{
		if ( futur < hope )  futur = hope;
	}

	return futur;
}

// Version précise et lente de Smooth.

float SmoothP(float actual, float hope, float time)
{
	float	futur, step;
	int		iter, i;

	if ( actual == hope )  return hope;

	if ( time < 0.001f )
	{
		futur = actual + (hope-actual)*time;
	}
	else
	{
		iter = (int)(time/0.001f);
		step = time/iter;
		futur = actual;
		for ( i=0 ; i<iter ; i++ )
		{
			futur += (hope-futur)*step;
		}
	}

	if ( hope > actual )
	{
		if ( futur > hope )  futur = hope;
	}
	if ( hope < actual )
	{
		if ( futur < hope )  futur = hope;
	}

	return futur;
}

// Version approximative et rapide de SmoothP.
// Sur une machine rapide, lorsque le nombre de FPS devient grand,
// les robots ont des mouvements d'amplitude trop grande en utilisant
// Smooth. Cette procédure Smoove essaye de corriger cela empiriquement,
// bien que cela soit mathématiquement faux !

float SmoothA(float actual, float hope, float time)
{
	float	futur;

	if ( actual == hope )  return hope;

	futur = actual + (hope-actual)*powf(time, 0.5f);  // sqr

	if ( hope > actual )
	{
		if ( futur > hope )  futur = hope;
	}
	if ( hope < actual )
	{
		if ( futur < hope )  futur = hope;
	}

	return futur;
}

// Transforme une progression pour obtenir un mouvement dou.

float Soft(float progress, int iter)
{
	int		i;

	for ( i=0 ; i<iter ; i++ )
	{
		progress = sinf(PI*1.5f+progress*PI)/2.0f+0.5f;
	}
	return progress;
}


// Fait reboudir un mouvement quelconque.

//	out
//	 |
//	1+------o-------o---
//	 |    o | o   o |  | bounce
//	 |   o  |   o---|---
//	 |  o   |       |
//	 | o    |       |
//	-o------|-------+----> progress
//	0|      |       1
//	 |<---->|middle

float Bounce(float progress, float middle, float bounce)
{
	if ( progress < middle )
	{
		progress = progress/middle;  // 0..1
		return 0.5f+sinf(progress*PI-PI/2.0f)/2.0f;
	}
	else
	{
		progress = (progress-middle)/(1.0f-middle);  // 0..1
		return (1.0f-bounce/2.0f)+sinf((0.5f+progress*2.0f)*PI)*(bounce/2.0f);
	}
}


// Retourne la couleur D3DCOLOR correspondante.

D3DCOLOR RetColor(int r, int g, int b)
{
	D3DCOLOR	color;

	color  = 0<<24;
	color |= r<<16;
	color |= g<<8;
	color |= b;

	return color;
}

// Retourne la couleur RGB correspondante.

void RetColor(D3DCOLOR intensity, int &r, int &g, int &b)
{
	r = (intensity>>16)&0xff;
	g = (intensity>>8 )&0xff;
	b = (intensity>>0 )&0xff;
}

// Retourne la couleur D3DCOLOR correspondante.

D3DCOLOR RetColor(float intensity)
{
	D3DCOLOR	color;

	if ( intensity <= 0.0f )  return 0x00000000;
	if ( intensity >= 1.0f )  return 0xffffffff;

	color  = (int)(intensity*255.0f)<<24;
	color |= (int)(intensity*255.0f)<<16;
	color |= (int)(intensity*255.0f)<<8;
	color |= (int)(intensity*255.0f);

	return color;
}

// Retourne la couleur D3DCOLOR correspondante.

D3DCOLOR RetColor(D3DCOLORVALUE intensity)
{
	D3DCOLOR	color;

	color  = (int)(intensity.a*255.0f)<<24;
	color |= (int)(intensity.r*255.0f)<<16;
	color |= (int)(intensity.g*255.0f)<<8;
	color |= (int)(intensity.b*255.0f);

	return color;
}

// Retourne la couleur D3DCOLORVALUE correspondante.

D3DCOLORVALUE RetColor(D3DCOLOR intensity)
{
	D3DCOLORVALUE	color;

	color.r = (float)((intensity>>16)&0xff)/256.0f;
	color.g = (float)((intensity>>8 )&0xff)/256.0f;
	color.b = (float)((intensity>>0 )&0xff)/256.0f;
	color.a = (float)((intensity>>24)&0xff)/256.0f;

	return color;
}


// Conversion RGB vers HSV.

void RGB2HSV(D3DCOLORVALUE src, ColorHSV &dest)
{
	float	min, max, delta;

	min = Min(src.r, src.g, src.b);
	max = Max(src.r, src.g, src.b);

	dest.v = max;  // intensité

	if ( max == 0.0f )
	{
		dest.s = 0.0f;  // saturation
		dest.h = 0.0f;  // teinte indéfinie !
	}
	else
	{
		delta = max-min;
		dest.s = delta/max;  // saturation

		if ( src.r == max )  // between yellow & magenta
		{
			dest.h = (src.g-src.b)/delta;
		}
		else if ( src.g == max )  // between cyan & yellow
		{
			dest.h = 2.0f+(src.b-src.r)/delta;
		}
		else  // between magenta & cyan
		{
			dest.h = 4.0f+(src.r-src.g)/delta;
		}

		dest.h *= 60.0f;  // en degrés
		if ( dest.h < 0.0f )  dest.h += 360.0f;
		dest.h /= 360.0f;  // 0..1
	}
}

// Conversion HSV vers RGB.

void HSV2RGB(ColorHSV src, D3DCOLORVALUE &dest)
{
	int		i;
	float	f,v,p,q,t;

	src.h = Norm(src.h)*360.0f;
	src.s = Norm(src.s);
	src.v = Norm(src.v);

	if ( src.s == 0.0f )  // saturation nulle ?
	{
		dest.r = src.v;
		dest.g = src.v;
		dest.b = src.v;  // gris
	}
	else
	{
		if ( src.h == 360.0f )  src.h = 0.0f;
		src.h /= 60.0f;
		i = (int)src.h;  // partie entière (0..5)
		f = src.h-i;     // partie fractionnaire

		v = src.v;
		p = src.v*(1.0f-src.s);
		q = src.v*(1.0f-(src.s*f));
		t = src.v*(1.0f-(src.s*(1.0f-f)));

		switch (i)
		{
			case 0:  dest.r=v; dest.g=t; dest.b=p;  break;
			case 1:  dest.r=q; dest.g=v; dest.b=p;  break;
			case 2:  dest.r=p; dest.g=v; dest.b=t;  break;
			case 3:  dest.r=p; dest.g=q; dest.b=v;  break;
			case 4:  dest.r=t; dest.g=p; dest.b=v;  break;
			case 5:  dest.r=v; dest.g=p; dest.b=q;  break;
		}
	}
}

