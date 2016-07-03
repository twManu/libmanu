
/*
current version:  may 9th, 2003

this file contains a class template for elements stored in a B-tree, and a
class for a B-tree node, which provides all the methods needed to search,
insert, or delete.  a sample "main" function is also provided to show
how to use the B-tree.
to understand the identifiers and comments, visualize the tree as having
its root node at the top of the diagram, its leaf nodes at the bottom of
the diagram, and each node as containing an array oriented horizontally,
with the smallest element on the left and the largest element on the right.
the zeroth element of a node contains only a subtree pointer, no key value
or payload.

a b-tree grows "upward", by splitting the root node when the node's capacity
is exceeded.  conversely, the depth of the tree is always reduced by merging
the last remaining element of the root node with the elements of its two
child nodes, so the tree contracts "from the top".

this code may be freely copied.
programmer: toucan@textelectric.net
algorithm and pseudo-code found in:
"fundamentals of data structures in pascal", by Horowitz and Sahni

there is a java applet on the web that draws a b-tree diagram and allows the
user to perform insertions and deletions, so you can see how it grows and shrinks,
at: http://www.mars.dti.ne.jp/~torao/program/structure/btree.html
*/

#include <iostream>
#include <string>
#include <vector>
#include <strstream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

using namespace std;
class Node;
Node* invalid_ptr = reinterpret_cast<Node*> (-1);
Node* null_ptr = reinterpret_cast<Node*> (0);
const int invalid_index = -1;
const int max_elements = 9;  // max elements in a node
// size limit for the array in a vector object.  best performance was
// at 800 bytes.
const int max_array_bytes = 800;

template<class key, class payload> class Element {
// contains a key value, a payload, and a pointer toward the subtree
// containing key values greater than this->m_key but lower than the
// key value of the next element to the right

public:
    key m_key;
    payload m_payload;
    Node* mp_subtree;

public:
    bool operator>   (Element& other) const { return m_key >  other.m_key; }
    bool operator<   (Element& other) const { return m_key <  other.m_key; }
    bool operator>=  (Element& other) const { return m_key >= other.m_key; }
    bool operator<=  (Element& other) const { return m_key <= other.m_key; }
    bool operator==  (Element& other) const { return m_key == other.m_key; }
    bool valid () const { return mp_subtree != invalid_ptr; }
    void invalidate () { mp_subtree = invalid_ptr; }
    Element& operator= (const Element& other) {
        m_key = other.m_key;
        m_payload = other.m_payload;
        mp_subtree = other.mp_subtree;
        return *this;
    }
    Element () { mp_subtree = null_ptr; }
    void dump ();
}; //______________________________________________________________________


template<class key, class payload> void Element<key, payload>::dump () {
    cout << "key=" << m_key << ", sub=" << mp_subtree << endl;
} //_______________________________________________________________________


/*****
  type-dependant
  Element of key string, payload string type
 *****/
typedef Element<string, string> Elem_StrStr;


class RootTracker;


/*****
  type-dependant
  Noed of element key string, payload string vector
 *****/
class Node {
protected:
    // locality of reference, beneficial to effective cache utilization,
    // is provided by a "vector" container rather than a "list"
    vector<Elem_StrStr> m_vector;
    // number of elements currently in m_vector, including the zeroth element
    // which has only a subtree, no key value or payload.
    int m_count;
    Node* mp_parent;
    bool is_leaf();
    bool vector_insert (Elem_StrStr& element);
    bool vector_insert_for_split (Elem_StrStr& element);
    bool split_insert (Elem_StrStr& element);
    bool vector_delete (Elem_StrStr& target);
    bool vector_delete (int target_pos);
    void insert_zeroth_subtree (Node* subtree);
    void set_debug();
    int key_count () { return m_count-1; }
    Elem_StrStr& largest_key () { return m_vector[m_count-1]; }
    Elem_StrStr& smallest_key () { return m_vector[1]; }
    Elem_StrStr& smallest_key_in_subtree();
    int index_has_subtree ();
    Node* right_sibling (int& parent_index_this);
    Node* left_sibling (int& parent_index_this);
    Node* rotate_from_left(int parent_index_this);
    Node* rotate_from_right(int parent_index_this);
    Node* merge_right (int parent_index_this);
    Node* merge_left (int parent_index_this);
    bool merge_into_root ();
    int minimum_keys ();
#ifdef _DEBUG
    Elem_StrStr debug[8];
#endif
public:
    Elem_StrStr& search (Elem_StrStr& desired, Node*& last_visited);
    bool tree_insert (Elem_StrStr& element);
    bool delete_element (Elem_StrStr& target);
    int delete_all_subtrees ();
    Node* find_root();
    // to return a reference when a search fails.
    static Elem_StrStr m_failure;
    // the root of the tree may change.  this attribute keeps it accessible.
    RootTracker& m_root;
    Elem_StrStr& operator[] (int i) { return m_vector[i]; }
    // node cannot be instantiated without a root tracker
    Node (RootTracker& root_track);
    void dump ();
}; //______________________________________________________________________


class RootTracker {
// all the node instances that belong to a given tree have a reference to one
// instance of RootTracker.  as the Node instance that is the root may change
// or the original root may be deleted, Node instances must access the root
// of the tree through this tracker, and update the root pointer when they
// perform insertions or deletions.  using a static attribute in the Node
// class to hold the root pointer would limit a program to just one B-tree.
protected:
    Node* mp_root;
public:
    RootTracker() { mp_root = null_ptr; }
    void set_root (Node* old_root, Node* new_root) {
        // ensure that the root is only being changed by a node belonging to the
        // same tree as the current root
        if (old_root != mp_root)
            throw "wrong old_root in RootTracker::set_root";
        else
            mp_root = new_root;
    }
    Node* get_root () { return mp_root; }

    ~RootTracker () {
        // safety measure
        if (mp_root) {
            mp_root->delete_all_subtrees();
            delete mp_root;
        }
    }
}; //_______________________________________________________________________



int Node::minimum_keys ()
{
    // minus 1 for the empty slot left for splitting the node
    int size = m_vector.size();
    int ceiling_func = (size-1)/2;
    if (ceiling_func*2 < size-1)
        ceiling_func++;
    return ceiling_func-1;  // for clarity, may increment then decrement
} //________________________________________________________________________


inline void Node::set_debug()
{
#ifdef _DEBUG
// the contents of an STL vector are not visible in the visual C++ debugger,
// so this function copies up to eight elements from the STL vector into
// a simple C++ array.
    for (int i=0; i<m_count && i<8; i++) {
        debug[i] = m_vector[i];
        if (m_vector[i].mp_subtree)
            m_vector[i].mp_subtree->set_debug();
    }
    for ( ; i<8; i++)
        debug[i] = m_failure;
#endif
} //________________________________________________________________________


void Node::insert_zeroth_subtree (Node* subtree) {
    m_vector[0].mp_subtree = subtree;
    m_vector[0].m_key = "";	//type-dependant
    m_count = 1;
    if (subtree)
        subtree->mp_parent = this;
} //_________________________________________________________________________


void Node::dump ()
{
// write out the keys in this node and all its subtrees, along with
// node adresses, for debugging purposes
	int i;
        if (this == m_root.get_root()) cout << "ROOT\n";
        cout << "\nthis=" << this << endl;
        cout << "parent=" << mp_parent << " count=" << m_count << endl;
        for (i=0; i<m_count; i++) m_vector[i].dump();
        for (i=0; i<m_count; i++)
		if (m_vector[i].mp_subtree)
			m_vector[i].mp_subtree->dump();
        cout << endl;
} //________________________________________________________________________


Node::Node (RootTracker& root_track)  : m_root(root_track)
{
// limit the size of the vector to 4 kilobytes max and 200 entries max.
        int num_elements = max_elements*sizeof(Elem_StrStr)<=(unsigned int)max_array_bytes ?
                           max_elements : max_array_bytes/sizeof(Elem_StrStr);
        if (num_elements < 6)  // in case key or payload is really huge
            num_elements = 6;
        m_vector.resize (num_elements);
        m_count = 0;
        mp_parent = 0;
        insert_zeroth_subtree (0);
} //________________________________________________________________________


Node* Node::find_root ()
{
    Node* current = this;
    while (current->mp_parent)
        current = current->mp_parent;
    return current;
} //__________________________________________________________________________


bool Node::is_leaf ()
{
    return m_vector[0].mp_subtree==0;
} //________________________________________________________________________


int Node::delete_all_subtrees ()
{
// return the number of nodes deleted
    int count_deleted = 0;
    for (int i=0; i< m_count; i++) {
        if (!m_vector[i].mp_subtree) continue;
        else if (m_vector[i].mp_subtree->is_leaf()) {
            delete m_vector[i].mp_subtree;
            count_deleted++;
        }
        else
            count_deleted += m_vector[i].mp_subtree->delete_all_subtrees();
    }
    return count_deleted;
} //_______________________________________________________________________


bool Node::vector_insert (Elem_StrStr& element) {
// this method merely tries to insert the argument into the current node.
// it does not concern itself with the entire tree.
// if the element can fit into m_vector, slide all the elements
// greater than the argument forward one position and copy the argument
// into the newly vacated slot, then return true.  otherwise return false.
// note: the tree_insert method will already have verified that the key
// value of the argument element is not present in the tree.
    if ((unsigned int)m_count >= m_vector.size()-1) // leave an extra slot for split_insert
        return false;
    int i;
    //move to latter slot for those greater than new coming element, inefficiency
    for( i=m_count; i>0 && m_vector[i-1]>element; --i )
	m_vector[i] = m_vector[i-1];
    if (element.mp_subtree) element.mp_subtree->mp_parent = this;
    m_vector[i] = element;
    m_count++;
    return true;
} //__________________________________________________________________



bool Node::vector_delete (Elem_StrStr& target) {
// delete a single element in the vector belonging to *this node.
// if the target is not found, do not look in subtrees, just return false.
    int target_pos = -1;
    int first = 1;
    int last = m_count-1;
    // perform binary search
    while (last-first > 1) {
        int mid = first+(last-first)/2;
        if (target>=m_vector[mid]) first = mid;
        else last = mid;
    }

    if (m_vector[first]==target) target_pos = first;
    else if (m_vector[last]==target) target_pos = last;
    else return false;

    // the element's subtree, if it exists, is to be deleted or re-attached
    // in a different function.  not a concern here.  just shift all the
    // elements in positions greater than target_pos.
    for (int i=target_pos; i<m_count; i++) m_vector[i] = m_vector[i+1];
    m_count--;
    return true;
} //____________________________________________________________________


bool Node::vector_delete (int target_pos) {
// delete a single element in the vector belonging to *this node.
// the element is identified by position, not value.
    if (target_pos<0 || target_pos>=m_count) return false;
    // the element's subtree, if it exists, is to be deleted or re-attached
    // in a different function.  not a concern here.  just shift all the
    // elements in positions greater than target_pos.
    for (int i=target_pos; i<m_count; i++) m_vector[i] = m_vector[i+1];
    m_count--;
    return true;
} //____________________________________________________________________


bool Node::vector_insert_for_split (Elem_StrStr& element) {
// this method insert an element that is in excess of the nominal capacity of
// the node, using the extra slot that always remains unused during normal
// insertions.  this method should only be called from split_insert()
    if ((unsigned int)m_count >= m_vector.size()) // error
        return false;
    int i = m_count;
    for( i=m_count; i>0 && m_vector[i-1]>element; --i )
	m_vector[i] = m_vector[i-1];
    if (element.mp_subtree) element.mp_subtree->mp_parent = this;
    m_vector[i] = element;
    m_count++;
    return true;
} //__________________________________________________________________


bool Node::split_insert (Elem_StrStr& element) {
    // split_insert should only be called if node is full
    if ((unsigned int)m_count != m_vector.size()-1)
        throw "bad m_count in split_insert";

    vector_insert_for_split (element);
    int split_point = m_count/2;
    if (2*split_point < m_count)  // perform the "ceiling function"
        split_point++;
    // new node receives the rightmost half of elements in *this node
    Node* new_node = new Node(m_root);
    Elem_StrStr upward_element = m_vector[split_point];
    new_node->insert_zeroth_subtree (upward_element.mp_subtree);
    upward_element.mp_subtree = new_node;
    // element that gets added to the parent of this node
    for (int i=1; i<m_count-split_point; i++)
        new_node->vector_insert(m_vector[split_point+i]);
    new_node->m_count = m_count-split_point;
    m_count = split_point;
    new_node->mp_parent = mp_parent;

    // now insert the new node into the parent, splitting it if necessary
    if (mp_parent && mp_parent->vector_insert(upward_element))
        return true;
    else if (mp_parent && mp_parent->split_insert(upward_element))
        return true;
    else if (!mp_parent) { // this node was the root
        Node* new_root = new Node(m_root);
        new_root->insert_zeroth_subtree(this);
        this->mp_parent = new_root;
        new_node->mp_parent = new_root;
        new_root->vector_insert (upward_element);
        m_root.set_root (m_root.get_root(),  new_root);
        new_root->mp_parent = 0;
    }
    return true;
}//__________________________________________________________________


bool Node::tree_insert (Elem_StrStr& element) {
    Node* last_visited_ptr = this;
    if (search(element, last_visited_ptr).valid())  // element already in tree
        return false;
    if (last_visited_ptr->vector_insert(element))
        return true;
    return last_visited_ptr->split_insert(element);
} //__________________________________________________________________


bool Node::delete_element (Elem_StrStr& target) {
// target is just a package for the key value.  the reference does not
// provide the address of the Elem instance to be deleted.

    // first find the node contain the Elem instance with the given key
    Node* node = 0;
    int parent_index_this = invalid_index;
    Elem_StrStr& found = search (target, node);
    if (!found.valid())
        return false;

    if (node->is_leaf() && node->key_count() > node->minimum_keys())
        return node->vector_delete (target);
    else if (node->is_leaf()) {
        node->vector_delete (target);
        // loop invariant: if _node_ is not null_ptr, it points to a node
        // that has lost an element and needs to import one from a sibling
        // or merge with a sibling and import one from its parent.
        // after an iteration of the loop, _node_ may become null or
        // it may point to its parent if an element was imported from the
        // parent and this caused the parent to fall below the minimum
        // element count.
        while (node) {
            // NOTE: the "this" pointer may no longer be valid after the first
            // iteration of this loop!!!
            if (node==node->find_root() && node->is_leaf())
                break;
            if (node==node->find_root() && !node->is_leaf()) // sanity check
                throw "node should not be root in delete_element loop";
            // is an extra element available from the right sibling (if any)

            Node* right = node->right_sibling(parent_index_this);
            if (right && right->key_count() > right->minimum_keys())
                node = node->rotate_from_right(parent_index_this);
            else {
                // is an extra element available from the left sibling (if any)
                Node* left = node->left_sibling(parent_index_this);
                if (left && left->key_count() > left->minimum_keys())
                    node = node->rotate_from_left(parent_index_this);
                else if (right)
                    node = node->merge_right(parent_index_this);
                else if (left)
                    node = node->merge_left(parent_index_this);
            }
        }
    }
    else {
        Elem_StrStr& smallest_in_subtree = found.mp_subtree->smallest_key_in_subtree();
        found.m_key = smallest_in_subtree.m_key;
        found.m_payload = smallest_in_subtree.m_payload;
        found.mp_subtree->delete_element (smallest_in_subtree);
    }
    return true;
} //___________________________________________________________________


Node* Node::rotate_from_right(int parent_index_this) {
    // new element to be added to this node
    Elem_StrStr underflow_filler = (*mp_parent)[parent_index_this+1];
    // right sibling of this node
    Node* right_sib = (*mp_parent)[parent_index_this+1].mp_subtree;
    underflow_filler.mp_subtree = (*right_sib)[0].mp_subtree;
    // copy the entire element
    (*mp_parent)[parent_index_this+1] = (*right_sib)[1];
    // now restore correct pointer
    (*mp_parent)[parent_index_this+1].mp_subtree = right_sib;
    vector_insert (underflow_filler);
    right_sib->vector_delete(0);
    (*right_sib)[0].m_key = "";
    (*right_sib)[0].m_payload = "";
    return null_ptr; // parent node still has same element count
} //_______________________________________________________________________


Node* Node::rotate_from_left(int parent_index_this) {
    // new element to be added to this node
    Elem_StrStr underflow_filler = (*mp_parent)[parent_index_this];
    // left sibling of this node
    Node* left_sib = (*mp_parent)[parent_index_this-1].mp_subtree;
    underflow_filler.mp_subtree = (*this)[0].mp_subtree;
    (*this)[0].mp_subtree = (*left_sib)[left_sib->m_count-1].mp_subtree;
    if ((*this)[0].mp_subtree)
        (*this)[0].mp_subtree->mp_parent = this;
    // copy the entire element
    (*mp_parent)[parent_index_this] = (*left_sib)[left_sib->m_count-1];
    // now restore correct pointer
    (*mp_parent)[parent_index_this].mp_subtree = this;
    vector_insert (underflow_filler);
    left_sib->vector_delete(left_sib->m_count-1);
    return null_ptr; // parent node still has same element count
} //_______________________________________________________________________


Node* Node::merge_right (int parent_index_this) {
// copy elements from the right sibling into this node, along with the
// element in the parent node vector that has the right sibling as it subtree.
// the right sibling and that parent element are then deleted
    Elem_StrStr parent_elem = (*mp_parent)[parent_index_this+1];
    Node* right_sib = (*mp_parent)[parent_index_this+1].mp_subtree;
    parent_elem.mp_subtree = (*right_sib)[0].mp_subtree;
    vector_insert (parent_elem);
    for (int i=1; i<right_sib->m_count; i++)
        vector_insert ((*right_sib)[i]);
    mp_parent->vector_delete (parent_index_this+1);
    delete right_sib;
    if (mp_parent==find_root() && !mp_parent->key_count()) {
        m_root.set_root(m_root.get_root(), this);
        delete mp_parent;
        mp_parent = 0;
        return null_ptr;
    }
    else if (mp_parent==find_root() && mp_parent->key_count())
        return null_ptr;
    if (mp_parent&& mp_parent->key_count() >= mp_parent->minimum_keys())
        return null_ptr; // no need for parent to import an element
    return mp_parent; // parent must import an element
} //_______________________________________________________________________


Node* Node::merge_left (int parent_index_this) {
// copy all elements from this node into the left sibling, along with the
// element in the parent node vector that has this node as its subtree.
// this node and its parent element are then deleted.
    Elem_StrStr parent_elem = (*mp_parent)[parent_index_this];
    parent_elem.mp_subtree = (*this)[0].mp_subtree;
    Node* left_sib = (*mp_parent)[parent_index_this-1].mp_subtree;
    left_sib->vector_insert (parent_elem);
    for (int i=1; i<m_count; i++)
        left_sib->vector_insert (m_vector[i]);
    mp_parent->vector_delete (parent_index_this);
    Node* parent_node = mp_parent;  // copy before deleting this node
    if (mp_parent==find_root() && !mp_parent->key_count()) {
        m_root.set_root(m_root.get_root(), left_sib);
        delete mp_parent;
        left_sib->mp_parent = null_ptr;
        delete this;
        return null_ptr;
    }
    else if (mp_parent==find_root() && mp_parent->key_count()) {
        delete this;
        return null_ptr;
    }
    delete this;
    if (parent_node->key_count() >= parent_node->minimum_keys())
        return null_ptr; // no need for parent to import an element
    return parent_node; // parent must import an element
} //_______________________________________________________________________


Node* Node::right_sibling (int& parent_index_this) {
    parent_index_this = index_has_subtree (); // for element with THIS as subtree
    if (parent_index_this == invalid_index)
        return 0;
    // now mp_parent is known not to be null
    if (parent_index_this >= mp_parent->m_count-1)
        return 0;  // no right sibling
    return mp_parent->m_vector[parent_index_this+1].mp_subtree;  // might be null
} //__________________________________________________________________________


Node* Node::left_sibling (int& parent_index_this)
{
    parent_index_this = index_has_subtree (); // for element with THIS as subtree
    if (parent_index_this == invalid_index)
        return 0;
    // now mp_parent is known not to be null
    if (parent_index_this==0)
        return 0;  // no left sibling
    return mp_parent->m_vector[parent_index_this-1].mp_subtree;  // might be null
} //____________________________________________________________________________


int Node::index_has_subtree ()
{
// return the element in this node's parent that has the "this" pointer as its subtree
    if (!mp_parent)
        return invalid_index;
    int first = 0;
    int last = mp_parent->m_count-1;
    while (last-first > 1) {
        int mid = first+(last-first)/2;
        if (smallest_key()>=mp_parent->m_vector[mid])
            first = mid;
        else
            last = mid;
    }
    if (mp_parent->m_vector[first].mp_subtree == this)
        return first;
    else if (mp_parent->m_vector[last].mp_subtree == this)
        return last;
    else
        throw "error in index_has_subtree";
} //__________________________________________________________________


Elem_StrStr& Node::smallest_key_in_subtree ()
{
    if (is_leaf()) return m_vector[1];
    else return m_vector[0].mp_subtree->smallest_key_in_subtree();
} //___________________________________________________________________



Elem_StrStr& Node::search (Elem_StrStr& desired, Node*& last_visited_ptr) {
    // the zeroth element of the vector is a special case (no key value or
    // payload, just a subtree).  the seach starts at the *this node, not
    // at the root of the b-tree.
    Node* current = this;
    if (!key_count())
        current = 0;
    while (current) {
        last_visited_ptr = current;
        // if desired is less than all values in current node
        if (current->m_count>1 && desired<current->m_vector[1])
            current = current->m_vector[0].mp_subtree;
        // if desired is greater than all values in current node
        else if (desired>current->m_vector[current->m_count-1])
            current = current->m_vector[current->m_count-1].mp_subtree;
        else {
            // binary search of the node
            int first = 1;
            int last = current->m_count-1;
            while (last-first > 1) {
                int mid = first+(last-first)/2;
                if (desired>=current->m_vector[mid])
                    first = mid;
                else
                    last = mid;
            }
            if (current->m_vector[first]==desired)
                return current->m_vector[first];
            if (current->m_vector[last]==desired)
                return current->m_vector[last];
            else if (current->m_vector[last]>desired)
                current = current->m_vector[first].mp_subtree;
            else
                current = current->m_vector[last].mp_subtree;
        }
    }

    return m_failure;
} //_____________________________________________________________________


static void QueryPerformanceFrequency(long long *freq) { *freq = 1000 * 1000; }
//return Usec value of current time
static void QueryPerformanceCounter(long long *val)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	*val = tv.tv_sec * 1000 * 1000 + tv.tv_usec;
}

// initialize static data at file scope

Elem_StrStr Node::m_failure = Elem_StrStr();

#define	MAX_COUNT	100
int main(int argc, char* argv[])
{
// the main function is just some code to test the b-tree.  it inserts 100,000 elements,
// then searches for each of them, then deletes them in reverse order (also tested in
// forward order) and searches for all 100,000 elements after each deletion to ensure that
// all remaining elements remain accessible.
    long long frequency, start, end, total;

    QueryPerformanceFrequency( &frequency );
    Node::m_failure.invalidate();
    Node::m_failure.m_key = "";
    Elem_StrStr elem;

    RootTracker tracker;  // maintains a pointer to the current root of the b-tree
    Node* root_ptr = new Node(tracker);
    tracker.set_root (null_ptr, root_ptr);
    vector<string> search_vect;
    // prepare the key strings
    search_vect.resize (MAX_COUNT);

    int i;
    for (i=0; i<MAX_COUNT; i++) {
        strstream stm;
        stm << i;
        stm >> search_vect[i];
    }
    cout << "finished preparing key strings\n";

/*
 * randomly insert a key and each needs be done once
 */
    //all false by default
    vector<bool> searched_vect;
    int searched_count;
    searched_vect.resize (MAX_COUNT);
    for( i=searched_count=0; i<MAX_COUNT; ++i ) searched_vect[i] = false;
    //init random
    QueryPerformanceCounter (&start);
    i = (int)start;
    cout << "Seeded with " << i << endl;
    srandom((unsigned int) i);
    while( searched_count!=MAX_COUNT ) {
        i = random() % MAX_COUNT;
        if( searched_vect[i] ) continue;	//skip if searched
        searched_vect[i] = true;
        elem.m_key = search_vect[i];
        elem.m_payload = search_vect[i] + " hi you";
        ++searched_count;
        cout << "Inserting element w/ key " << search_vect[i] << " ..." << MAX_COUNT-searched_count << " left\n";
        tracker.get_root()->tree_insert(elem);
    }
    cout << "finished inserting elements\n";
    tracker.get_root()->dump();

/*
 * randomly search a key and each needs be done once
 */
    //all false by default
    for( i=searched_count=0; i<MAX_COUNT; ++i ) searched_vect[i] = false;
    Node * last;
    while( searched_count!=MAX_COUNT ) {
        Elem_StrStr desired;
        i = random() % MAX_COUNT;
        if( searched_vect[i] ) continue;	//skip if searched
        searched_vect[i] = true;
        ++searched_count;
        desired.m_key = search_vect[i];
        Elem_StrStr& result = tracker.get_root()->search (desired, last);
        cout << result.m_payload << " ..." << MAX_COUNT-searched_count << " left\n";
    }
    cout << "finished searching for elements\n\n";

/*
 * randomly delete a key and each needs be done once
 */
    //all false by default
    for( i=searched_count=0; i<MAX_COUNT; ++i ) searched_vect[i] = false;
    while( searched_count!=MAX_COUNT ) {
        Elem_StrStr target;
        i = random() % MAX_COUNT;
        if( searched_vect[i] ) continue;	//skip if searched
        searched_vect[i] = true;
        ++searched_count;
        target.m_key = search_vect[i];
	cout << "Deleting element w/ key " << target.m_key << " ..." << MAX_COUNT-searched_count << " left\n";
        tracker.get_root()->delete_element (target);
    }

    QueryPerformanceCounter (&end);
    total = (end-start)/(frequency/1000);
    cout << endl << "total millisec for " << (int)MAX_COUNT << " elements: " << (int)total << endl;

    cout << "after deletion\n";
    tracker.get_root()->dump();
    //getchar();

    return 0;
}
