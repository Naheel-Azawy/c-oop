#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

// utils -----------------------------------------------------------------------

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define YEL   "\x1B[33m"
#define BLU   "\x1B[34m"
#define MAG   "\x1B[35m"
#define CYN   "\x1B[36m"
#define WHT   "\x1B[37m"
#define RESET "\x1B[0m"

#define error(s, ...) { fprintf(stderr, RED s RESET "\n", ##__VA_ARGS__); exit(EXIT_FAILURE); }
#define errorif(c, s, ...) if (c) error(s, ##__VA_ARGS__);
#define errorifc(c, s, ...) if (c) error("Error: " s " " #c , ##__VA_ARGS__);

#define info(s, ...) { fprintf(stdout, BLU s RESET "\n", ##__VA_ARGS__); }
#define infoif(c, s, ...) if (c) info(s, ##__VA_ARGS__);

char* strconcat(int num_args, ...) {
	int strsize = 0;
	va_list ap;
	va_start(ap, num_args);
	for (int i = 0; i < num_args; i++)
		strsize += strlen(va_arg(ap, char*));

	char* res = malloc(strsize+1);
	strsize = 0;
	va_start(ap, num_args);
	for (int i = 0; i < num_args; i++) {
		char* s = va_arg(ap, char*);
		strcpy(res+strsize, s);
		strsize += strlen(s);
	}
	va_end(ap);
	res[strsize] = '\0';

	return res;
}

char* itostr(int i) {
	int length = snprintf(NULL, 0, "%d", i);
	char* str = malloc(length + 1);
	snprintf(str, length + 1, "%d", i);
	return str;
}

char* ltostr(long i) {
	int length = snprintf(NULL, 0, "%d", i);
	char* str = malloc(length + 1);
	snprintf(str, length + 1, "%d", i);
	return str;
}

char* ftostr(double i) {
	int length = snprintf(NULL, 0, "%f", i);
	char* str = malloc(length + 1);
	snprintf(str, length + 1, "%f", i);
	return str;
}

#define btostr(b) (b ? "true" : "false")

// utils (END) -----------------------------------------------------------------

// main header -----------------------------------------------------------------

// returns pointer to parent class
#define parent_class(self, self_class)                          \
  ( ( ( class_base* )(((object*)self)->cls) )->parent_pointer )

// Type base -------------------------------------------------------------------

typedef enum {
              class,
              iface
} type;

typedef struct _type_base type_base;

struct _type_base {
	type  base_type;
	void* ptr;
	char* name;
};

#define type_base_init(_name, type) {                                   \
    ((type_base*)_name ## _ ## type ## _)->base_type = type;            \
    ((type_base*)_name ## _ ## type ## _)->ptr = (void*) _name ## _ ## type ## _; \
    ((type_base*)_name ## _ ## type ## _)->name = #_name;               \
  }

#define NULL_class_ NULL
#define NULL_iface_ NULL

#define class_base_init(_name, _parent) {                               \
    type_base_init(_name, class);                                       \
    ((class_base*)_name ## _class_)->parent_pointer = (class_base*) _parent ## _class_; \
  }

#define iface_base_init(_name, _parent) {                               \
    type_base_init(_name, iface);                                       \
    ((iface_base*)_name ## _iface_)->parent_pointer = (iface_base*) _parent ## _iface_; \
  }

#define set_instance_class(name) ((object*)self)->cls = (class_base*) name ## _class_

// Type base (END) -------------------------------------------------------------

// Interface base --------------------------------------------------------------

typedef struct _iface_base iface_base;
typedef struct _iface_container iface_container;

struct _iface_base {
	type_base   base;
	iface_base* parent_pointer;
};

struct _iface_container {
	unsigned int count;
	iface_base** ifaces; // interfaces of parents must be included here
};

#define class_implements(_name, count, ...)                             \
	((class_base*)_name ## _class_)->ifaces_cont = ( (count == 0) ? NULL : iface_container_new(count, ##__VA_ARGS__) )

iface_container* iface_container_new(unsigned int count, ...) {
	if (count == 0) return NULL;
	va_list ap;
	iface_container* res = malloc(sizeof(iface_container));
	iface_base**     arr = malloc(sizeof(iface_base*)*count);
	res->count = count;
	res->ifaces = arr;
	va_start(ap, count);
	for (int i = 0; i < count; i++) {
		iface_base* s = va_arg(ap, iface_base*);
		arr[i] = s;
	}
	va_end(ap);

	return res;
}

// Interface base (END) --------------------------------------------------------

// Class base ------------------------------------------------------------------

typedef struct _class_base class_base;

struct _class_base {
	type_base        base;
	class_base*      parent_pointer;
	iface_container* ifaces_cont;
};

// Class base (END) ------------------------------------------------------------

// class object ----------------------------------------------------------------

/*
  class object {
	char* to_string() {
  return "Not implemented";
	}
  }
*/

typedef struct _object object;
typedef struct _object_class object_class;

struct _object {
	class_base* cls;
};

struct _object_class {
	class_base parent;
	char*      (*to_string)(object* self);
	int        (*compare_to)(object* self, object* o);
};

#define object_to_string(self) ((object_class*)((object*)self)->cls)->to_string((object*)self)
char* object_real_to_string(object* self) {
	return strconcat(5, "[ ", ((type_base*)self->cls)->name, " ", ltostr((long)self->cls), " ]");
}

#define object_compare_to(self, o) ((object_class*)((object*)self)->cls)->compare_to(self, o)
int object_real_compare_to(object* self, object* o) {
	return self == o ? 0 : (self > o ? 1 : -1);
}

object_class* object_class_ = NULL;

static void object_class_init() {
	if (object_class_ == NULL) {
		object_class_ = malloc(sizeof(object_class));
		class_base_init(object, NULL);
		class_implements(object, 0);
		object_class_->to_string = object_real_to_string;
		object_class_->compare_to = object_real_compare_to;
	}
}

#define object_new() _object_new(sizeof(object))
object* _object_new(size_t size) {
	object_class_init();
	object* self = malloc(size);
	set_instance_class(object);
	return self;
}


// class object (END) ----------------------------------------------------------


// TODO use hashmaps
#define get_iface(self, iface) ((typeof(iface ## _iface_)) _get_iface((object*)self, (iface_base*)iface ## _iface_))
iface_base* _get_iface(object* obj, iface_base* iface) {
	errorifc(obj == NULL, "couldn't get %s iface", ((type_base*)iface)->name);
	iface_container* cont = ((class_base*)obj->cls)->ifaces_cont;
	errorifc(cont == NULL, "couldn't get %s iface", ((type_base*)iface)->name);
	iface_base** arr  = cont->ifaces;
	errorifc(arr == NULL, "couldn't get %s iface", ((type_base*)iface)->name);
	void* target = ((type_base*)iface)->ptr;
	iface_base* p;
	for (int i = 0; i<cont->count; i++) {
		p = (iface_base*) ((type_base*)arr[i])->ptr;
		while (p != NULL) {
			if (p == target) return arr[i];
			p = p->parent_pointer;
		}
	}
	error("couldn't get %s iface", ((type_base*)iface)->name);
	return NULL;
}

#define instanceofc(obj, t) instanceof(obj, t ## _class_)
#define instanceofi(obj, t) instanceof(obj, t ## _iface_)

#define instanceof(obj, t) _instanceof((object*)obj, (type_base*)t)
bool _instanceof(object* obj, type_base* target) {
	if (obj == NULL) return false;
	void* m = ((type_base*)obj->cls)->ptr;
	void* t = target->ptr;
	if (m == t) return true;
	if (target->base_type == class) {
		class_base* p = ((class_base*)obj->cls)->parent_pointer;
		while (p != NULL) {
			if (((type_base*)p)->ptr == t) return true;
			p = p->parent_pointer;
		}
	} else if (target->base_type == iface) {
		return _get_iface(obj, (iface_base*)target) != NULL;
	}
	return false;
}

// main header (END) -----------------------------------------------------------


// class string ----------------------------------------------------------------

/*
  class string : object {

	int length;
	char* str;

	string(char* s) {
  length = strlen(s);
  str = s;
	}

	char* to_string() {
  return str;
	}

	void concat(string s) {
  strconcat(str, s);
  length += s.length;
	}

  }
*/

typedef struct _string string;
typedef struct _string_class string_class;

struct _string {
	object parent;
	int    length;
	char*  str;
};

struct _string_class {
	object_class parent;
	void         (*concat)(string* self, string* s);
};

char* string_real_to_string(object* self) {
	return ((string*)self)->str;
}

#define string_concat(self, s) ((string_class*)((object*)self)->cls)->concat(self, s)
void string_real_concat(string* self, string* s) {
	self->str = strconcat(2, self->str, s->str);
	self->length += s->length;
}

string_class* string_class_ = NULL;

static void string_class_init() {
	if (string_class_ == NULL) {
		object_class_init();
		string_class_ = malloc(sizeof(string_class));
		string_class_->parent = *object_class_;
		class_base_init(string, object);
		class_implements(string, 0);
		((object_class*)string_class_)->to_string = string_real_to_string;
		string_class_->concat = string_real_concat;
	}
}

#define string_new(s) _string_new(sizeof(string), s)
string* _string_new(size_t size, char* s) {
	string_class_init();
	string* self = (string*) _object_new(size);
	set_instance_class(string);

	self->length = strlen(s);
	self->str = s;

	return self;
}

// class string (END) ----------------------------------------------------------


// class human -----------------------------------------------------------------

/*
  class human : object {

	static int counter = 0;

	string name;
	int age;
	human(string name, int age) {
  this.name = name;
  this.age = age;
  counter++;
	}

	char* to_string() {
  return "human: name="+name+" age="+age;
	}

	void sayHi() {
  printf("Hi, I'm %s\n", name);
	}
  }
*/

typedef struct _human human;
typedef struct _human_class human_class;

struct _human {
	object parent;
	string name;
	int    age;
};

struct _human_class {
	object_class  parent;
	void (*say_hi)(human* self);
};

int human_counter = 0;

char* human_real_to_string(object* self) {
	return strconcat(4, "name=", ((human*)self)->name.str, ", age=", itostr(((human*)self)->age));
}

#define human_say_hi(self) ((human_class*)((object*)self)->cls)->say_hi(self)
void human_real_say_hi(human* self) {
	printf("Hi, I'm %s\n", self->name.str);
}

human_class* human_class_ = NULL;

static void human_class_init() {
	if (human_class_ == NULL) {
		object_class_init();
		human_class_ = malloc(sizeof(human_class));
		human_class_->parent = *object_class_;
		class_base_init(human, object);
		class_implements(human, 0);
		((object_class*)human_class_)->to_string = human_real_to_string;
		human_class_->say_hi = human_real_say_hi;
	}
}

#define human_new(name, age) _human_new(sizeof(human), name, age)
human* _human_new(size_t size, char* name, int age) {
	human_class_init();
	human* self = (human*) _object_new(size);
	set_instance_class(human);

	self->name = *string_new(name);
	self->age = age;
	human_counter++;

	return self;
}

// class human (END) -----------------------------------------------------------


// class student ---------------------------------------------------------------

/*
  class student : human {
	double gpa;
	student(stirng name, int age, double gpa) {
  base(name, age);
  this.gpa = gpa;
	}
	void study() {
  print("%s is studying\n", name);
	}
  }
*/

typedef struct _student student;
typedef struct _student_class student_class;

struct _student {
	human  parent;
	double gpa;
};

struct _student_class {
	human_class  parent;
	void         (*study)(student* self);
};

student_class* student_class_ = NULL;

char* student_real_to_string(object* self) {
	return strconcat(3,
                   ((object_class*)parent_class(self, student))->to_string((object*)self),
                   ", gpa=", ftostr(((student*)self)->gpa));
}

#define student_study(self) ((student_class*)(((object*)self)->cls))->study(self)
void student_real_study(student* self) {
	printf("%s is studying\n", ((human*)self)->name.str);
}

static void student_class_init() {
	if (student_class_ == NULL) {
		human_class_init();
		student_class_ = malloc(sizeof(student_class));
		student_class_->parent = *human_class_;
		class_base_init(student, human);
		class_implements(student, 0);
		((object_class*)student_class_)->to_string = student_real_to_string;
		student_class_->study = student_real_study;
	}
}

#define student_new(name, age, gpa) _student_new(sizeof(student), name, age, gpa)
student* _student_new(size_t size, char* name, int age, double gpa) {
	student_class_init();
	student* self = (student*) _human_new(size, name, age);
	set_instance_class(student);

	self->gpa = gpa;

	return self;
}

// class student (END) ---------------------------------------------------------

// interface iterator ----------------------------------------------------------

#define T void*

typedef object iterator;
typedef struct _iterator_iface iterator_iface;

struct _iterator_iface {
	iface_base parent;
	T          (*next)(object* self);
	bool       (*hasnext)(object* self);
};

iterator_iface* iterator_iface_ = NULL;

static void iterator_iface_init() {
	if (iterator_iface_ == NULL) {
		iterator_iface_ = malloc(sizeof(iterator_iface));
		iface_base_init(iterator, NULL);
	}
}

#define iterator_next(self) get_iface(self, iterator)->next((object*)self)
#define iterator_hasnext(self) get_iface(self, iterator)->hasnext((object*)self)

//#undef T

// interface iterator (END) ----------------------------------------------------

// interface iterable ----------------------------------------------------------

#define T void*

typedef object iterable;
typedef struct _iterable_iface iterable_iface;

struct _iterable_iface {
	iface_base parent;
	iterator*  (*iterator)(object* self);
};

iterable_iface* iterable_iface_ = NULL;

static void iterable_iface_init() {
	if (iterable_iface_ == NULL) {
		iterable_iface_ = malloc(sizeof(iterable_iface));
		iface_base_init(iterable, NULL);
	}
}

#define iterable_iterator(self) get_iface(self, iterable)->iterator((object*)self)

#define iterable_foreach(iterable, type, item)                    \
	iterator* it = iterable_iterator(iterable); /*TODO hide this*/  \
	for (type* item = ((type*)iterator_next(it));                   \
       item != NULL;                                              \
       item = ((type*)iterator_next(it)))

//#undef T

// interface iterable (END) ----------------------------------------------------

// interface collection --------------------------------------------------------

#define T void*

typedef object collection;
typedef struct _collection_iface collection_iface;

struct _collection_iface {
	iterable_iface parent;
	bool           (*add)(collection* self, T o);
	void           (*clear)(collection* self);
	bool           (*contains)(collection* self, T o);
	bool           (*is_empty)(collection* self);
	bool           (*remove)(collection* self, T o);
	size_t         (*size)(collection* self);
};

collection_iface* collection_iface_ = NULL;

static void collection_iface_init() {
	iterable_iface_init();
	if (collection_iface_ == NULL) {
		collection_iface_ = malloc(sizeof(collection_iface));
		iface_base_init(collection, iterable);
	}
}

#define collection_iterator(self)    iterable_iterator(self)
#define collection_add(self, o)      get_iface(self, collection)->add((collection*)self, (T)o)
#define collection_clear(self)       get_iface(self, collection)->clear((collection*)self)
#define collection_contains(self, o) get_iface(self, collection)->contains((collection*)self, (T)o)
#define collection_is_empty(self)    get_iface(self, collection)->is_empty((collection*)self)
#define collection_remove(self, o)   get_iface(self, collection)->remove((collection*)self, (T)o)
#define collection_size(self)        get_iface(self, collection)->size((collection*)self)

//#undef T

// interface collection (END) --------------------------------------------------

// interface list --------------------------------------------------------------

#define T void*

typedef object list;
typedef struct _list_iface list_iface;

struct _list_iface {
	collection_iface parent;
	T                (*get)(list* self, int index);
	void             (*set)(list* self, int index, T val);
};

list_iface* list_iface_ = NULL;

static void list_iface_init() {
	collection_iface_init();
	if (list_iface_ == NULL) {
		list_iface_ = malloc(sizeof(list_iface));
		iface_base_init(list, collection);
	}
}

#define list_iterator(self)    iterable_iterator(self)
#define list_add(self, o)      collection_add(self, o)
#define list_clear(self)       collection_clear(self)
#define list_contains(self, o) collection_contains(self, o)
#define list_is_empty(self)    collection_is_empty(self)
#define list_remove(self, o)   collection_remove(self, o)
#define list_size(self)        collection_size(self)
#define list_get(self, index)  get_iface(self, list)->get((list*)self, index)
#define list_set(self, index, val) get_iface(self, list)->set((list*)self, index, (T)val)

//#undef T

// interface list (END) --------------------------------------------------------

// class linkedlist ------------------------------------------------------------

#define T void*

typedef struct _linkedlist_node linkedlist_node;

typedef struct _linkedlist linkedlist;
typedef struct _linkedlist_class linkedlist_class;

typedef struct _linkedlist_myiterator linkedlist_myiterator;
typedef struct _linkedlist_myiterator_class linkedlist_myiterator_class;

struct _linkedlist_node {
	T                item;
	linkedlist_node* next;
};

struct _linkedlist {
	object           parent;
	linkedlist_node* first;
	linkedlist_node* last;
	int              length;
};

struct _linkedlist_class {
	object_class  parent;
	bool          (*is_empty)(linkedlist* self);
	void          (*add)(linkedlist* self, T item);
	void          (*set)(linkedlist* self, int position, T item);
	T             (*get)(linkedlist* self, int position);
	void          (*delete)(linkedlist* self, int position);
};

// struct node -------------------------------------------------------------

linkedlist_node* linkedlist_node_new(T item, linkedlist_node* next) {
	linkedlist_node* self = malloc(sizeof(linkedlist_node));
	self->item = item;
	self->next = next;
	return self;
}

// struct node (END) -------------------------------------------------------

// class linkedlist_myiterator -----------------------------------------------

struct _linkedlist_myiterator {
	object           parent;
	linkedlist*      outer; // for nested classes
	linkedlist_node* cur;
};

struct _linkedlist_myiterator_class {
	object_class  parent;
};

#define linkedlist_myiterator_real_next(self)     \
	_linkedlist_myiterator_real_next((object*)self)
T _linkedlist_myiterator_real_next(object* _self) {
	linkedlist_myiterator* self = (linkedlist_myiterator*) _self;
	linkedlist_node* n = self->cur;
	if (n == NULL) return NULL;
	self->cur = self->cur->next;
	return n->item;
}

#define linkedlist_myiterator_real_hasnext(self)      \
	_linkedlist_myiterator_real_hasnext((object*)self)
bool _linkedlist_myiterator_real_hasnext(object* _self) {
	linkedlist_myiterator* self = (linkedlist_myiterator*) _self;
	return self->cur != NULL;
}

linkedlist_myiterator_class* linkedlist_myiterator_class_ = NULL;
iterator_iface* linkedlist_myiterator_class_iterator_iface_ = NULL;

static void linkedlist_myiterator_class_init() {
	if (linkedlist_myiterator_class_ == NULL) {
		object_class_init();
		iterator_iface_init();
		// init linkedlist_myiterator_class_iterator_iface_
		linkedlist_myiterator_class_iterator_iface_ = malloc(sizeof(iterator_iface));
		*linkedlist_myiterator_class_iterator_iface_ = *((iterator_iface*) iterator_iface_);
		linkedlist_myiterator_class_iterator_iface_->next = _linkedlist_myiterator_real_next;
		linkedlist_myiterator_class_iterator_iface_->hasnext = _linkedlist_myiterator_real_hasnext;
		// init linkedlist_myiterator_class_
		linkedlist_myiterator_class_ = malloc(sizeof(linkedlist_myiterator_class_));
		linkedlist_myiterator_class_->parent = *object_class_;
		class_base_init(linkedlist_myiterator, object);
		class_implements(linkedlist_myiterator, 1, linkedlist_myiterator_class_iterator_iface_);
	}
}

#define linkedlist_myiterator_new(outer) _linkedlist_myiterator_new(sizeof(linkedlist_myiterator), outer)
linkedlist_myiterator* _linkedlist_myiterator_new(size_t size, linkedlist* outer) {
	linkedlist_myiterator_class_init();
	linkedlist_myiterator* self = (linkedlist_myiterator*) _object_new(size);
	set_instance_class(linkedlist_myiterator);

	self->outer = outer;
	self->cur = outer->first;

	return self;
}

// class linkedlist_myiterator (END) -----------------------------------------

/*char* linkedlist_real_to_string(object* self) {
	return "Not implemented yet"; // TODO
  }*/


iterator* linkedlist_iterable_real_iterator(object* self) {
	return (iterator*) linkedlist_myiterator_new((linkedlist*) self);
}

bool linkedlist_collection_real_add(collection* _self, T item) {
	linkedlist* self = (linkedlist*) _self;
	if (self->first == NULL) {
		self->first = linkedlist_node_new(item, NULL);
		self->last = self->first;
	} else {
		linkedlist_node* t = self->last;
		linkedlist_node* n = linkedlist_node_new(item, NULL);
		t->next = n;
		self->last = n;
	}
	self->length++;
	return true;
}

void linkedlist_collection_real_clear(collection* _self) {
	linkedlist* self = (linkedlist*) _self;
	iterable_foreach (self, T, i) {
		list_remove(self, i);
	}
}

bool linkedlist_collection_real_contains(collection* _self, T o) {
	linkedlist* self = (linkedlist*) _self;
	iterable_foreach (self, T, i) {
		if (i == o) return true;
	}
	return false;
}

bool linkedlist_collection_real_is_empty(collection* _self) {
	linkedlist* self = (linkedlist*) _self;
	return self->length == 0;
}

bool linkedlist_collection_real_remove(collection* _self, T o) {
	linkedlist* self = (linkedlist*) _self;
	// TODO implement
	if (o == NULL) return false;
	if (list_is_empty(self)) return false;
	linkedlist_node* cur = self->first;
	linkedlist_node* prev;
	for (int i = 0; i<list_size(self); i++) {
		if (object_compare_to(cur->item, o) == 0) break;
		prev = cur;
		cur = cur->next;
	}
	if (cur == self->first) {
		self->first = self->first->next;
		free(cur);
	} else if (cur != NULL) {
		prev->next = cur->next;
		if (cur == self->last)
			self->last = prev;
		free(cur);
		return true;
	}
	return false;
}

size_t linkedlist_collection_real_size(collection* _self) {
	linkedlist* self = (linkedlist*) _self;
	return self->length;
}

// non virtual
linkedlist_node* linkedlist_get_helper(linkedlist* self, int position) {
	if (list_is_empty(self)) return NULL;
	if (position >= self->length) return NULL;
	linkedlist_node* cur = self->first;
	for (int i = 0; i<position; i++) {
		cur = cur->next;
	}
	return cur;
}

void linkedlist_list_real_set(list* _self, int position, T item) {
	linkedlist* self = (linkedlist*) _self;
	linkedlist_node* n = linkedlist_get_helper(self, position);
	if (n != NULL) n->item = item;
}

T linkedlist_list_real_get(list* _self, int position) {
	linkedlist* self = (linkedlist*) _self;
	linkedlist_node* n = linkedlist_get_helper(self, position);
	return n == NULL ? NULL : n->item;
}

linkedlist_class* linkedlist_class_ = NULL;
list_iface* linkedlist_class_list_iface_ = NULL;

static void linkedlist_class_init() {
	if (linkedlist_class_ == NULL) {
		object_class_init();
		list_iface_init();
		// init linkedlist_class_list_iface_
		linkedlist_class_list_iface_ = malloc(sizeof(list_iface));
		*linkedlist_class_list_iface_ = *((list_iface*) list_iface_);
		((iterable_iface*)linkedlist_class_list_iface_)->iterator = linkedlist_iterable_real_iterator;
		((collection_iface*)linkedlist_class_list_iface_)->add = linkedlist_collection_real_add;
		((collection_iface*)linkedlist_class_list_iface_)->clear = linkedlist_collection_real_clear;
		((collection_iface*)linkedlist_class_list_iface_)->contains = linkedlist_collection_real_contains;
		((collection_iface*)linkedlist_class_list_iface_)->is_empty = linkedlist_collection_real_is_empty;
		((collection_iface*)linkedlist_class_list_iface_)->remove = linkedlist_collection_real_remove;
		((collection_iface*)linkedlist_class_list_iface_)->size = linkedlist_collection_real_size;
		((list_iface*)linkedlist_class_list_iface_)->get = linkedlist_list_real_get;
		((list_iface*)linkedlist_class_list_iface_)->set = linkedlist_list_real_set;
		// init linkedlist_class
		linkedlist_class_ = malloc(sizeof(linkedlist_class));
		linkedlist_class_->parent = *object_class_;
		class_base_init(linkedlist, object);
		class_implements(linkedlist, 1, linkedlist_class_list_iface_);
		//((object_class*)linkedlist_class)->to_string = linkedlist_real_to_string;
	}
}

#define linkedlist_new() _linkedlist_new(sizeof(linkedlist))
linkedlist* _linkedlist_new(size_t size) {
	linkedlist_class_init();
	linkedlist* self = (linkedlist*) _object_new(size);
	set_instance_class(linkedlist);

	self->first = NULL;
	self->last = NULL;
	self->length = 0;

	return self;
}

//#undef T

// class linkedlist (END) ------------------------------------------------------

int main(void) {

	object* o = object_new();
	printf("o to_string(): %s\n", object_to_string(o));

	human* h = human_new("Hooman", 20);
	printf("h to_string(): %s\n", object_to_string(h));

	student* s = student_new("Stoodant", 30, 3.5);
	printf("s to_string(): %s\n", object_to_string(s));

	linkedlist* l = linkedlist_new();
	printf("l to_string(): %s\n", object_to_string(l));

	list_add(l, human_new("Hooman 0", 20));
	list_add(l, human_new("Hooman 1", 21));
	list_add(l, human_new("Hooman 2", 22));
	list_add(l, human_new("Hooman 3", 23));

	printf("iterable_foreach:\n");
	iterable_foreach (l, human, h) {
		printf("%s\n", object_to_string(h));
	}

	printf("classic for:\n");
	for (int i = 0; i<list_size(l); i++) {
		printf("%s\n", object_to_string((human*)list_get(l, i)));
	}

	printf("checking l instance:\n");
	printf(
         "l is linkedlist = %s\n"
         "l is collection = %s\n"
         "l is iterable = %s\n"
         "l is object = %s\n"
         "l is human = %s\n",
         btostr(instanceofc(l, linkedlist)),
         btostr(instanceofi(l, collection)),
         btostr(instanceofi(l, iterable)),
         btostr(instanceofc(l, object)),
         btostr(instanceofc(l, human))
         );

	return 0;
}
