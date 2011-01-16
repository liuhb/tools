#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <pthread.h>


using namespace std;

class noncopyable
{
protected:
	noncopyable() {}
	~noncopyable() {}
private:  // emphasize the following members are private
	noncopyable( const noncopyable& );
	const noncopyable& operator=( const noncopyable& );
};


template < class T > 
class mulqueue  : public noncopyable
{
public:
	mulqueue(const int _len = 100) : len(_len) 
	{
		data = new T[len];
		assert(data != NULL);
		head = 0;
		tail = 0;
		pthread_mutex_init(&q_cs, NULL);
		pthread_cond_init(&q_cv, NULL);
	}
	~mulqueue() {
		delete data;
		pthread_mutex_destroy(&q_cs);
		pthread_cond_destroy(&q_cv);
	}
	bool enqueue(const T & _val) {
		bool ret = true;
		int  next;

		pthread_mutex_lock(&q_cs);
		next = tail + 1;
		if ( next >= len ) {
			next = 0;
		}
		if ( next == head ) {
			ret = false;
		} else {
			data[tail] = _val;
			tail = next;
		}

		pthread_cond_signal(&q_cv);
		pthread_mutex_unlock(&q_cs);

		return ret;
	}

	T    dequeue() {
		T  _t;
		
		pthread_mutex_lock(&q_cs);
		while ( head == tail ) {
			pthread_cond_wait(&q_cv, &q_cs);
		}

		_t = data[head ++];
		if ( head >= len ) {
			head = 0;
		}
		pthread_mutex_unlock(&q_cs);

		return _t;

	}
	
	
private:
	int len;
	int head,tail;
	T*  data;

	pthread_mutex_t q_cs;
	pthread_cond_t q_cv;
};


void process(void * data)
{
	mulqueue<int> * que = (mulqueue<int>*) data;

	while(true) {
		fprintf(stderr,"print %d \n",que->dequeue());
	}
}

int main(int argc, char ** argv)
{
	mulqueue<int> q(1000);
	pthread_t tid;		
	pthread_create(&tid, NULL, (void*(*)(void*))process, (void*)&q);
	pthread_create(&tid, NULL, (void*(*)(void*))process, (void*)&q);
	pthread_create(&tid, NULL, (void*(*)(void*))process, (void*)&q);
	pthread_create(&tid, NULL, (void*(*)(void*))process, (void*)&q);
	pthread_create(&tid, NULL, (void*(*)(void*))process, (void*)&q);
	pthread_create(&tid, NULL, (void*(*)(void*))process, (void*)&q);
	pthread_create(&tid, NULL, (void*(*)(void*))process, (void*)&q);
	
	sleep(2);
	for ( int i = 1 ; i < 20000 ; i ++ ) {
		while(!q.enqueue(i)) sleep(1);
	}
	sleep(10);

	return 0;
}
