#include <string/mystring.h>

Strin* newStrin(){
	Strin* s=NULL;
	s=malloc(sizeof(Strin));
	if(s==NULL){
		perror("Strin malloc failed\n");
		return NULL;
	}
	s->msize=STRIN_BUCKET;
	s->slen=0;
	s->s=malloc(STRIN_BUCKET);
	if(s->s==NULL){
		free(s);
		perror("Strin malloc failed\n");
		return NULL;
	}
	s->s[0]=0;
	return s;
}

int addcToStrin(Strin* s,char c){
	if(s->slen+2 < s->msize){
		s->s[s->slen]=c;
		s->s[s->slen+1]=0;
		s->slen++;
	}else{
		Strin* cache;
		s->msize+=STRIN_BUCKET;
		cache=realloc(s->s, STRIN_BUCKET + s->msize);
		if(cache==NULL){
			free(s->s);
			free(s);
			return FAIL;
		}
		s->s=(char*)cache;
		addcToStrin(s,c);
	}
	return OK;
}

char* retChar(Strin* s){
	return s->s;
}
