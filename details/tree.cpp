/*This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.*/
/* Michael Welsch (c) 2018 */

#include "tree.hpp" // back reference for header only use
#include <algorithm>
#include <cmath>
#include <functional>
#include <sstream>
#include <vector>
#if defined BOOST_SERIALIZATION_NVP
#define SERIALIZATION_NVP2(name, variable)          \
    boost::serialization::make_nvp(name, variable)
#define SERIALIZATION_NVP(variable) BOOST_SERIALIZATION_NVP(variable)
#else
#define SERIALIZATION_NVP2(name, variable) (variable)
#define SERIALIZATION_NVP(variable) (variable)
#endif

#ifdef BOOST_SERIALIZATION_SPLIT_MEMBER
#define SERIALIZE_SPLIT_MEMBERS BOOST_SERIALIZATION_SPLIT_MEMBER
#else
#ifndef SERIALIZE_SPLIT_MEMBERS
#define SERIALIZE_SPLIT_MEMBERS()
#endif
#endif

namespace metric_space {

/*
  __ \          _|                |  |         \  |        |         _)
  |   |   _ \  |     _` |  |   |  |  __|      |\/ |   _ \  __|   __|  |   __|
  |   |   __/  __|  (   |  |   |  |  |        |   |   __/  |    |     |  (
 ____/  \___| _|   \__,_| \__,_| _| \__|     _|  _| \___| \__| _|    _| \___|

*/
    template <typename Container> struct L2_Metric_STL {
        typedef typename Container::value_type result_type;
        static_assert(std::is_floating_point<result_type>::value,
                      "T must be a float type");

        result_type operator()(const Container &a, const Container &b) const {
            result_type sum = 0;
            for (auto it1 = a.begin(), it2 = b.begin();
                 it1 != a.end() || it2 != b.end(); ++it1, ++it2) {
                sum += (*it1 - *it2) * (*it1 - *it2);
            }
            return std::sqrt(sum);
        }
    };

/*
  \    |             |
   \   |   _ \    _` |   _ \
   |\  |  (   |  (   |   __/
  _| \_|  \___/ \__,_| \___|

  nodes of the tree
*/
    template <class recType, class Metric> class Node {
        Tree<recType, Metric> * tree_ptr = nullptr;
    public:
        using Distance = typename Tree<recType, Metric>::Distance;

        //    friend Tree<recType, Metric>;
        Node(Tree<recType, Metric> * ptr, Distance base = Tree<recType, Metric>().base) : tree_ptr(ptr), base(base) {}
        ~Node() {
            for (auto p : children) {
                delete p;
            }
        }
        //    typedef Tree<recType, Metric>::Node NodeType;
        // typedef std::shared_ptr<Tree<recType, Metric>::Node> Node_ptr;
        typedef Node<recType, Metric> *Node_ptr;
        //  typedef typename std::result_of<Metric(recType, recType)>::type Distance;

        // private:
        Distance base;
        recType data; // data record associated with the node
        recType center_of_mass;

        Node_ptr parent = nullptr;      // parent of current node
        std::vector<Node_ptr> children; // list of children
        int level = 0;                  // current level of the node
        Distance parent_dist = 0; // upper bound of distance to any of descendants
        unsigned ID = 0;          // unique ID of current node

        //    mutable std::shared_timed_mutex mut; // lock for current node
    public:
        unsigned get_ID() const { return ID; }
        void set_ID(const unsigned v) { ID = v; }
        const recType get_data() const { return data; }
        void set_data(const recType &r) { data = r; }
        Node_ptr get_parent() const { return parent; }
        void set_parent(Node_ptr node) { parent = node; }
        std::vector<Node_ptr> &get_children() { return children; }
        int get_level() const { return level; }
        void set_level(int l) { level = l; }
        Distance get_parent_dist() const { return parent_dist; }
        void set_parent_dist(const Distance &d) { parent_dist = d; }

        Distance covdist(); // covering distance of subtree at current node
        Distance sepdist(); // separating distance between nodes at current level

        Distance
        dist(const recType &pp) const;   // distance between this node and point pp
        Distance dist(Node_ptr n) const; // distance between this node and node n

        Node_ptr
        setChild(const recType &p,
                 int new_id = -1); // insert a new child of current node with point p
        Node_ptr setChild(Node_ptr p,
                          int new_id = -1); // // insert the subtree p as child of
        // current node (erase or reordering)

        // setting iterators for children access in loops
        typename std::vector<Node_ptr>::const_iterator begin() const {
            return children.cbegin();
        }
        typename std::vector<Node_ptr>::const_iterator end() const {
            return children.cend();
        }
        typename std::vector<Node_ptr>::iterator begin() { return children.begin(); }
        typename std::vector<Node_ptr>::iterator end() { return children.end(); }
        std::vector<Node_ptr> descendants() {
            std::vector<Node_ptr> result;
            Node_ptr curNode = this;
            std::stack<Node_ptr> nodeStack;

            nodeStack.push(curNode);
            while (nodeStack.size() > 0) {
                curNode = nodeStack.top();
                nodeStack.pop();
                // f(curNode); // .. and callback each node.
                result.push_back(curNode);
                for (const auto &child : *curNode) {
                    nodeStack.push(child);
                }
            }
            return result;
        }
        template <typename Archive> void serialize(Archive &ar, const unsigned int) {
            ar &SERIALIZATION_NVP(base) & SERIALIZATION_NVP(level) &
                SERIALIZATION_NVP(parent_dist) & SERIALIZATION_NVP(ID) &
                SERIALIZATION_NVP(data);
        }
    };

    template <typename recType, typename Metric> struct SerializedNode {
        using NodeType = Node<recType, Metric>;
        NodeType *node = nullptr;
        bool is_null = true;
        bool has_children = false;

        SerializedNode() : node(nullptr), is_null(true), has_children(false) {}
        ~SerializedNode() {}
        SerializedNode(NodeType *n)
            : node(n), is_null((node == nullptr) ? true : false),
              has_children(!is_null && node->children.empty() ? false : true) {}

        template <typename Archive> void save(Archive &ar, const unsigned int) const {
            ar << SERIALIZATION_NVP(is_null);
            if (!is_null) {
                // ar << *node << has_children;
                ar << SERIALIZATION_NVP2("node", *node)
                   << SERIALIZATION_NVP(has_children);
            }
        }
        template <typename Archive> void load(Archive &ar, const unsigned int) {
            ar >> SERIALIZATION_NVP(is_null);
            if (!is_null) {
                try {
                    node = new NodeType(nullptr);
                    ar >> SERIALIZATION_NVP2("node", *node);

                    ar >> SERIALIZATION_NVP(has_children);
                } catch (...) {
                    delete node;
                    node = nullptr;
                    throw;
                }
            }
        }
        SERIALIZE_SPLIT_MEMBERS();
        // BOOST_SERIALIZATION_SPLIT_MEMBER()
    };
/*
   \  |             |           _ _|                    |                                |           |   _)
    \ |   _ \    _` |   _ \       |   __ `__ \   __ \   |   _ \  __ `__ \    _ \  __ \   __|   _` |  __|  |   _ \   __ \
  |\  |  (   |  (   |   __/       |   |   |   |  |   |  |   __/  |   |   |   __/  |   |  |    (   |  |    |  (   |  |   |
 _| \_| \___/  \__,_| \___|     ___| _|  _|  _|  .__/  _| \___| _|  _|  _| \___| _|  _| \__| \__,_| \__| _| \___/  _|  _|
                                                _|
*/

/*** covering distance of subtree at current node ***/
    template <class recType, class Metric>
    typename Node<recType, Metric>::Distance Node<recType, Metric>::covdist() {
        return std::pow(base, level);
    }

/*** separating distance between nodes at current level ***/
    template <class recType, class Metric>
    typename Node<recType, Metric>::Distance Node<recType, Metric>::sepdist() {
        return 2 * std::pow(base, level - 1);
    }

/*** distance between current node and point pp ***/
    template <class recType, class Metric>
    typename Node<recType, Metric>::Distance
    Node<recType, Metric>::dist(const recType &pp) const {
        return tree_ptr->metric(data, pp);
    }

/*** distance between current node and node n ***/
    template <class recType, class Metric>
    typename Node<recType, Metric>::Distance
    Node<recType, Metric>::dist(Node_ptr n) const {
        return tree_ptr->metric(data, n->data);
    }

/*** insert a new child of current node with point p ***/
    template <class recType, class Metric>
    Node<recType, Metric> *Node<recType, Metric>::setChild(const recType &p,
                                                           int new_id) {
        Node_ptr temp(new Node<recType, Metric>());
        temp->data = p;
        temp->level = level - 1;
        temp->parent_dist = 0;
        temp->ID = new_id;
        temp->parent = this;
        children.push_back(temp);
        return temp;
    }

/*** insert the subtree p as child of current node ***/
    template <class recType, class Metric>
    Node<recType, Metric> *Node<recType, Metric>::setChild(Node_ptr p,
                                                           int /*new_id*/) {
        if (p->level != level - 1) {
            Node_ptr current = p;
            std::stack<Node_ptr> travel;
            current->level = level - 1;
            travel.push(current);

            while (travel.size() > 0) {
                current = travel.top();
                travel.pop();

                for (const auto &child : *current) {
                    child->level = current->level - 1;
                    travel.push(child);
                }
            }
        }

        p->parent = this;
        children.push_back(p);
        return p;
    }

    /*
       __ __|                    _ _|                    |                                |           |   _)
          |   __|  _ \   _ \       |   __ `__ \   __ \   |   _ \  __ `__ \    _ \  __ \   __|   _` |  __|  |   _ \   __ \
          |   |     __/   __/      |   |   |   |  |   |  |   __/  |   |   |   __/  |   |  |    (   |  |    |  (   |  |   |
         _| _|   \___| \___|    ___|  _|  _|  _|  .__/  _| \___| _|  _|  _| \___| _|  _| \__| \__,_| \__| _| \___/  _|  _|
                                                 _|
    */

/*
    __|                   |                   |
  (       _ \     \  (_-<   _|   _| |  |   _|   _|   _ \   _|
  \___| \___/ _| _| ___/  \__| _|  \_,_| \__| \__| \___/ _|

*/

/*** constructor: empty tree **/
    template <class recType, class Metric>
    Tree<recType, Metric>::Tree(int truncate /*=-1*/, Metric d) : metric_(d) {
        root = NULL;
        min_scale = 1000;
        max_scale = 0;
        truncate_level = truncate;
        N = 0;
    }

/*** constructor: with a signal data record **/
    template <class recType, class Metric>
    Tree<recType, Metric>::Tree(const recType &p, int truncateArg /*=-1*/, Metric d)
        : metric_(d) {
        min_scale = 1000;
        max_scale = 0;
        truncate_level = truncateArg;
        N = 1;

        root = std::make_unique<NodeType>();
        root->data = p;
        root->level = 0;
        root->parent_dist = 0;
        root->ID = 0;
    }

/*** constructor: with a vector data records **/
    template <class recType, class Metric>
    Tree<recType, Metric>::Tree(const std::vector<recType> &p,
                                int truncateArg /*=-1*/, Metric d)
        : metric_(d) {
        min_scale = 1000;
        max_scale = 0;
        truncate_level = truncateArg;
        N = 1;

        root = new NodeType(this); // std::make_unique<NodeType>();
        root->data = p[0];
        root->level = 0;
        root->parent_dist = 0;
        root->ID = 0;

        for (std::size_t i = 1; i < p.size(); ++i) {
            insert(p[i]);
        }
    }

/*** default deconstructor **/
    template <class recType, class Metric> Tree<recType, Metric>::~Tree() {
        delete root;
    }

    /*
                |     __|  |    _)  |      |                 _ )        _ \ _)       |
 (_-<   _ \   _| _|  (       \   |  |   _` |   _| -_)    \   _ \  |  |  |  | | (_-<   _|   _` |    \    _|   -_)
 ___/ \___/ _| \__| \___| _| _| _| _| \__,_| _| \___| _| _| ___/ \_, | ___/ _| ___/ \__| \__,_| _| _| \__| \___|
                                                                 ___/
    */
    template <class recType, class Metric>
    template <typename pointOrNodeType>
    std::tuple<std::vector<int>,
               std::vector<typename Tree<recType, Metric>::Distance>>
    Tree<recType, Metric>::sortChildrenByDistance(Node_ptr p,
                                                  pointOrNodeType x) const {
        auto num_children = p->children.size();
        std::vector<int> idx(num_children);
        std::iota(std::begin(idx), std::end(idx), 0);
        std::vector<Distance> dists(num_children);
        for (unsigned i = 0; i < num_children; ++i) {
            dists[i] = p->children[i]->dist(x);
        }
        auto comp_x = [&dists](int a, int b) { return dists[a] < dists[b]; };
        std::sort(std::begin(idx), std::end(idx), comp_x);
        return std::make_tuple(idx, dists);
    }

/*
  _ _|                      |
   |      \  (_-<   -_)   _| _|
  ___| _| _| ___/ \___| _| \__|
*/
    template <class recType, class Metric>
    std::size_t Tree<recType, Metric>::insert_if(const std::vector<recType> &p,
                                                 Distance treshold) {
        std::size_t inserted = 0;
        for (const auto &rec : p) {
            if (root->dist(rec) > treshold) {
                insert(rec);
                inserted++;
            }
        }
        return inserted;
    }
    template <class recType, class Metric>
    bool Tree<recType, Metric>::insert_if(const recType &p, Distance treshold) {
        if (root->dist(p) > treshold) {
            insert(p);
            return true;
        }
        return false;
    }

/*** vector of data record insertion  **/
    template <class recType, class Metric>
    bool Tree<recType, Metric>::insert(const std::vector<recType> &p) {
        for (const auto &rec : p) {
            insert(rec);
            //            print();
            //            std::cout << "-------------------" << std::endl;
        }
        return true;
    }

/*** data record insertion **/
    template <class recType, class Metric>
    bool Tree<recType, Metric>::insert(const recType &x) {
        std::unique_lock<std::shared_timed_mutex> lk(global_mut);
        (void)lk; // prevent AppleCLang warning;

        Node_ptr node = new NodeType(this);
        node->data = x;
        node->set_level(0);
        node->set_parent_dist(0);
        node->set_ID(N++);
        node->set_parent(nullptr);

        // root insertion
        if (root == NULL) {
            root = node;
        } else {
            root = insert(root, node);
            return true;
        }
        return false;
    }
/*** data record insertion **/
    template <class recType, class Metric>
    Node<recType, Metric> *Tree<recType, Metric>::insert(Node_ptr p, Node_ptr x) {
        Node_ptr result;

        // normal insertion
        if (p->dist(x) > p->covdist()) {
            // global_mut.unlock_shared(); // FIXME: this is not atomic
            // global_mut.lock();          //
            while (p->dist(x) > base * p->covdist() / (base - 1)) {
                Node_ptr current = p;
                Node_ptr parent = NULL;
                while (current->children.size() > 0) {
                    parent = current;
                    current = current->children.back();
                }
                if (parent != NULL) {
                    parent->children.pop_back();
                    current->set_level(p->get_level() + 1);
                    current->children.push_back(p);
                    p->parent = current;
                    p->parent_dist = p->dist(current);
                    p = current;
                    p->parent = nullptr;
                    p->parent_dist = 0;
                } else {
                    p->level += 1;
                }
            }
            x->level = p->level + 1;
            x->parent = nullptr;
            x->children.push_back(p);
            // x->ID = N++;
            p->parent_dist = p->dist(x);
            p->parent = x;
            p = x;
            max_scale = p->level;
            result = p;
            // result = true;
            // global_mut.unlock();         // FIXME: this is not atomic
            // global_mut.lock_shared();    //
        } else {
            result = insert_(p, x);
        }
        // global_mut.unlock_shared();
        return result;
    }

/*** recursive insertion part ***/
// template <class recType, class Metric>
// Node<recType,Metric>* Tree<recType, Metric>::insert_(Node_ptr p, Node_ptr x)
// {
//   //  std::cout << "insert_("  << x << ")"<< std::endl;
//     bool result = false;
//     bool flag = true;

//     // p->mut.lock_shared();
//     unsigned num_children = p->children.size(); // for later check, if there
//     is a change during the next steps

//     //auto[idx, dists] = sortChildrenByDistance(p, x); // can't wait C++17 ^^
//     auto idx__dists = sortChildrenByDistance(p, x);
//     auto idx = std::get<0>(idx__dists);
//     auto dists = std::get<1>(idx__dists);
//     for (const auto &q_idx : idx)
//     {
//         Node_ptr q = p->children[q_idx];

//         Distance d = dists[q_idx];

//         if (d <= q->covdist())
//         {
//             if (q->parent_dist < d)
//             {
//                 q->parent_dist = d;
//             }
//             // p->mut.unlock_shared();
//             auto q1 = insert_(q, x);
//             p->children[q_idx] = q1;
//             return p;
//         }
//     }
//     return rebalance(p,x);
// }

    template <class recType, class Metric>
    inline auto Tree<recType, Metric>::findAnyLeaf() -> Node_ptr {
        Node_ptr current = root;
        while (!current->children.empty()) {
            current = current->children.back();
        }
        return current;
    }

    template <class recType, class Metric>
    void Tree<recType, Metric>::extractNode(Node_ptr node) {
        Node_ptr parent = node->parent;
        if (parent == nullptr)
            return;
        unsigned num_children = parent->children.size();
        for (unsigned i = 0; i < num_children; ++i) {
            if (parent->children[i] == node) {
                parent->children[i] = parent->children.back();
                parent->children.pop_back();
                break;
            }
        }
        node->parent = nullptr;
    }

/*
    -_)   _| _` | (_-<   -_)
  \___| _| \__,_| ___/ \___|
*/
    template <class recType, class Metric>
    bool Tree<recType, Metric>::erase(const recType &p) {
        bool ret_val = false;
        // find the best node to inser
        std::unique_lock<std::shared_timed_mutex> lk(global_mut);
        (void)lk; // prevent AppleCLang warning

        std::pair<Node_ptr, Distance> result(root, root->dist(p));
        nn_(root, result.second, p, result);

        if (result.second <= 0.0) {
            Node_ptr node_p = result.first;
            Node_ptr parent_p = node_p->get_parent();

            if (node_p == root) {
                if (node_p->get_children().empty()) {
                    delete root;
                    root = nullptr;
                    N--;
                    return true;
                }
                auto leaf = findAnyLeaf();
                extractNode(leaf);
                leaf->set_level(root->get_level());
                root = leaf;
                leaf->children.assign(node_p->children.begin(), node_p->children.end());
                for (auto l : leaf->get_children()) {
                    l->set_parent(leaf);
                }
                ret_val = true;
                N--;
                node_p->children.clear();
                delete node_p;
            }

            else {
                // erase node from parent's list of child
                unsigned num_children = parent_p->children.size();
                for (unsigned i = 0; i < num_children; ++i) {
                    if (parent_p->children[i] == node_p) {
                        parent_p->children[i] = parent_p->children.back();
                        parent_p->children.pop_back();
                        break;
                    }
                }
                // insert each child of the node in new root again.
                for (Node_ptr q : node_p->children) {
                    root = Tree<recType, Metric>::insert_(root, q);
                }
                node_p->children.clear();
                delete node_p;
                N--;
                ret_val = true;
            }
        }
        return ret_val;
    }

/*

   \     \
  _| _| _| _|

  Nearest Neighbour
*/
    template <class recType, class Metric>
    typename Tree<recType, Metric>::Node_ptr
    Tree<recType, Metric>::nn(const recType &p) const {
        std::shared_lock<std::shared_timed_mutex> lk(global_mut);
        (void)lk;

        std::pair<Node_ptr, Distance> result(root, root->dist(p));
        nn_(root, result.second, p, result);
        return result.first;
    }

    template <class recType, class Metric>
    void Tree<recType, Metric>::nn_(Node_ptr current, Distance dist_current,
                                    const recType &p,
                                    std::pair<Node_ptr, Distance> &nn) const {

        if (dist_current < nn.second) // If the current node is the nearest neighbour
        {
            nn.first = current;
            nn.second = dist_current;
        }

        auto idx__dists = sortChildrenByDistance(current, p);
        auto idx = std::get<0>(idx__dists);
        auto dists = std::get<1>(idx__dists);
        // Distance max_dist = 0;
        // for(std::size_t i = 0; i < idx.size();i++) {
        //     if(p->children[i])
        // }
        for (const auto &child_idx : idx) {
            Node_ptr child = current->children[child_idx];
            Distance dist_child = dists[child_idx];

            if (nn.second > dist_child - child->parent_dist)
                nn_(child, dist_child, p, nn);
        }
    }

/*
  |
  | /     \     \
  _ \_ \ _| _| _| _|

  k-Nearest Neighbours */
    template <class recType, class Metric>
    std::vector<std::pair<typename Tree<recType, Metric>::Node_ptr,
                          typename Tree<recType, Metric>::Distance>>
    Tree<recType, Metric>::knn(const recType &queryPt, unsigned numNbrs) const {
        std::shared_lock<std::shared_timed_mutex> lk(global_mut);
        (void)lk;

        using NodePtr = typename Tree<recType, Metric>::Node_ptr;
        // Do the worst initialization
        std::pair<NodePtr, Distance> dummy(nullptr,
                                           std::numeric_limits<Distance>::max());
        // List of k-nearest points till now
        std::vector<std::pair<NodePtr, Distance>> nnList(numNbrs, dummy);

        // Call with root
        Distance dist_root = root->dist(queryPt);
        std::size_t nnSize = 0;
        nnSize = knn_(root, dist_root, queryPt, nnList, nnSize);
        if (nnSize < nnList.size()) {
            nnList.resize(nnSize);
        }
        return nnList;
    }
    template <class recType, class Metric>
    std::size_t
    Tree<recType, Metric>::knn_(Node_ptr current, Distance dist_current,
                                const recType &p,
                                std::vector<std::pair<Node_ptr, Distance>> &nnList,
                                std::size_t nnSize) const {
        if (dist_current <
            nnList.back()
            .second) // If the current node is eligible to get into the list
        {
            auto comp_x = [](std::pair<Node_ptr, Distance> a,
                             std::pair<Node_ptr, Distance> b) {
                              return a.second < b.second;
                          };
            std::pair<Node_ptr, Distance> temp(current, dist_current);
            nnList.insert(std::upper_bound(nnList.begin(), nnList.end(), temp, comp_x),
                          temp);
            nnList.pop_back();
            nnSize++;
        }

        auto idx__dists = sortChildrenByDistance(current, p);
        auto idx = std::get<0>(idx__dists);
        auto dists = std::get<1>(idx__dists);

        for (const auto &child_idx : idx) {
            Node_ptr child = current->children[child_idx];
            Distance dist_child = dists[child_idx];
            if (nnList.back().second > dist_child - child->parent_dist)
                nnSize = knn_(child, dist_child, p, nnList, nnSize);
        }
        return nnSize;
    }

/*

    _| _` |    \    _` |   -_)
  _| \__,_| _| _| \__, | \___|  ____/
  Range Neighbours Search
*/
    template <class recType, class Metric>
    std::vector<std::pair<typename Tree<recType, Metric>::Node_ptr,
                          typename Tree<recType, Metric>::Distance>>
    Tree<recType, Metric>::rnn(const recType &queryPt, Distance distance) const {

        std::vector<std::pair<Node_ptr, Distance>>
            nnList; // List of nearest neighbors in the rnn

        Distance dist_root = root->dist(queryPt);
        rnn_(root, dist_root, queryPt, distance, nnList); // Call with root

        return nnList;
    }
    template <class recType, class Metric>
    void Tree<recType, Metric>::rnn_(
        Node_ptr current, Distance dist_current, const recType &p,
        Distance distance,
        std::vector<std::pair<Node_ptr, Distance>> &nnList) const {

        if (dist_current <
            distance) // If the current node is eligible to get into the list
        {
            std::pair<Node_ptr, Distance> temp(current, dist_current);
            nnList.push_back(temp);
        }

        // auto[idx, dists] = sortChildrenByDistance(current, p);
        auto idx__dists = sortChildrenByDistance(current, p);
        auto idx = std::get<0>(idx__dists);
        auto dists = std::get<1>(idx__dists);

        for (const auto &child_idx : idx) {
            Node_ptr child = current->children[child_idx];
            Distance dist_child = dists[child_idx];
            if (distance > dist_child - child->parent_dist)
                rnn_(child, dist_child, p, distance, nnList);
        }
    }

/*
  _)
  (_-<  | _  /   -_)
  ___/ _| ___| \___|
  tree size
*/
    template <class recType, class Metric> size_t Tree<recType, Metric>::size() {
        std::shared_lock<std::shared_timed_mutex> lk(global_mut);
        (void)lk;
        return size_t(N);
    }

/*
   |        \ \   /           |
  _|   _ \ \ \ /  -_)   _|   _|   _ \   _|
  \__| \___/  \_/ \___| \__| \__| \___/ _|
  export to vector
*/
    template <class recType, class Metric>
    std::vector<recType> Tree<recType, Metric>::toVector() {
        std::shared_lock<std::shared_timed_mutex> lk(global_mut);
        (void)lk;

        std::vector<std::pair<recType, int>> zipped;

        std::stack<Node_ptr> stack;
        Node_ptr current;
        stack.push(root);
        while (stack.size() > 0) {
            current = stack.top();
            stack.pop();
            zipped.push_back(
                std::make_pair(current->data, current->ID)); // Add to dataset
            for (const auto &child : *current)
                stack.push(child);
        }

        // Sort the vector by index
        std::sort(std::begin(zipped), std::end(zipped),
                  [&](const auto &a, const auto &b) { return a.second < b.second; });

        std::vector<recType> data(zipped.size());
        for (int i = 0; i < zipped.size(); ++i) {
            data[i] = zipped[i].first;
        }
        return data;
    }

    template <class recType, class Metric>
    recType Tree<recType, Metric>::operator[](size_t id) {
        // iterate through tree with stack
        std::stack<Node_ptr> stack;
        Node_ptr current = root;
        stack.push(root);
        while (stack.size() > 0) {
            current = stack.top();
            stack.pop();
            if (current->ID == id)
                break;
            for (const auto &child : *current)
                stack.push(child);
        }
        return current->data;
    }

/*
  _ \                __ __|            |
  |   |  -_) \ \ /      |   _ \   _ \  | (_-<
  ___/ \___|  \_/      _| \___/ \___/ _| ___/
  debugging functions
*/

// get root level == max_level
    template <class recType, class Metric> int Tree<recType, Metric>::levelSize() {
        std::shared_lock<std::shared_timed_mutex> lk(global_mut);
        (void)lk;
        return root->level;
    }
    template <class recType, class Metric>
    std::map<int, unsigned> Tree<recType, Metric>::print_levels() {
        std::shared_lock<std::shared_timed_mutex> lk(global_mut);
        (void)lk;
        std::map<int, unsigned> level_count;
        std::stack<Node_ptr> stack;
        Node_ptr curNode;
        stack.push(root);
        while (stack.size() > 0) {
            curNode = stack.top();
            stack.pop();
            std::cout << "level: " << curNode->level << ",  node ID: " << curNode->ID
                      << ",  child ids: ";
            for (std::size_t i = 0; i < curNode->children.size(); ++i) {
                std::cout << curNode->children[i]->ID << ", ";
            }
            std::cout << std::endl;

            level_count[curNode->level]++;

            // Now push the children
            for (const auto &child : *curNode)
                stack.push(child);
        }
        return level_count;
    }

    template <class recType, class Metric>
    bool Tree<recType, Metric>::check_covering() const {
        std::shared_lock<std::shared_timed_mutex> lk(global_mut);
        (void)lk;
        bool result = true;
        std::stack<Node_ptr> stack;
        Node_ptr curNode;
        if (root == nullptr)
            return true;
        stack.push(root);
        while (stack.size() > 0) {
            curNode = stack.top();
            stack.pop();

            // Check covering for the current -> children pair
            for (const auto &child : *curNode) {
                stack.push(child);
                if (curNode->dist(child) > curNode->covdist()) {
                    std::cout << "covering ill here (" << curNode->get_ID() << ") --> ("
                              << child->get_ID() << ") dist < covdist "
                              << curNode->dist(child) << " < " << curNode->covdist()
                              << " level:" << curNode->get_level() << std::endl;
                    result = false;
                }
            }
        }

        return result;
    }


    template <class recType, class Metric>
    void Tree<recType, Metric>::print() const {
        if (root != nullptr)
            print_(root, std::cout);
        else {
            std::cout << "Empty tree" << std::endl;
        }
    }
    template <class recType, class Metric>
    void Tree<recType, Metric>::print(std::ostream &ostr) const {
        std::shared_lock<std::shared_timed_mutex> lk(global_mut);
        (void)lk;

        if (root != nullptr)
            print_(root, ostr);
        else {
            ostr << "Empty tree" << std::endl;
        }
    }

    template <class recType, class Metric>
    void Tree<recType, Metric>::print_(NodeType *node_p, std::ostream &ostr) const {
        static std::string depth2;

        static char depth[2056];
        static int di = 0;

        auto push = [&](char c) {
                        depth[di++] = ' ';
                        depth[di++] = c;
                        depth[di++] = ' ';
                        depth[di++] = ' ';
                        depth[di] = 0;
                    };

        auto pop = [&]() { depth[di -= 4] = 0; };
        //    recType pd;
        // if (node_p->parent) {
        //     //        auto  pd = node_p->parent->data;
        //     std::cout << "(" << node_p->ID << ',' << node_p->parent_dist << ','
        //               << node_p->level << ',' << node_p->data << ","
        //               << node_p->parent->ID << ")" << std::endl;
        //     // std::cout << "(" << node_p->level<< ',' << node_p->data << ")" <<
        //     // std::endl;
        // } else {
        //     // std::cout << "(" <<   node_p->level<< ',' <<  node_p->data << ")" <<
        //     // std::endl;
        //     std::cout << "(" << node_p->ID << ',' << node_p->parent_dist << ','
        //               << node_p->level << ',' << node_p->data << ","
        //               << " no_parent )" << std::endl;
        // }
        ostr << "(" << node_p->ID << ")" << std::endl;
        if (node_p->children.empty())
            return;

        for (std::size_t i = 0; i < node_p->children.size(); ++i) {
            ostr << depth;
            if (i < node_p->children.size() - 1) {
                ostr << " ├──";
                push('|');
            } else {
                ostr << " └──";
                push(' ');
            }
            print_(node_p->children[i], ostr);
            pop();
        }
    }

/*** traverse the tree from root and do something with every node ***/
    template <class recType, class Metric>
    void Tree<recType, Metric>::traverse(const std::function<void(Node_ptr)> &f) {
        std::shared_lock<std::shared_timed_mutex> lk(global_mut);
        (void)lk;

        // iterate though the tree...
        Node_ptr curNode = root;
        std::stack<Node_ptr> nodeStack;

        nodeStack.push(curNode);
        while (nodeStack.size() > 0) {
            curNode = nodeStack.top();
            nodeStack.pop();
            f(curNode); // .. and callback each node.
            for (const auto &child : *curNode) {
                nodeStack.push(child);
            }
        }
    }

    template <class recType, class Metric>
    void Tree<recType, Metric>::traverse_child(
        const std::function<void(Node_ptr)> &f) {
        std::shared_lock<std::shared_timed_mutex> lk(global_mut);
        (void)lk;
        // iterate though the tree...
        std::stack<Node_ptr> nodeStack;
        Node_ptr curNode = root;
        nodeStack.push(curNode);
        while (nodeStack.size() > 0) {
            curNode = nodeStack.top();
            nodeStack.pop();
            for (const auto &child : *curNode) {
                nodeStack.push(child); //.. and callback all child nodes.
                f(child);
            }
        }
        return;
    }

// template<typename recType, class Metric>
// template <class Serializer>
// inline
// void Tree<recType,Metric>::serialize_node(Node_ptr node, std::ostringstream &
// ostr, Serializer & serializer) {
//   if(!node->children.empty()) {
//     ostr << node->ID << " " << node->level << " " << node->parent_dist
//          << " t" << serializer(node->data) << " ";
//   } else {
//     ostr << node->ID << " " << node->level << " " << node->parent_dist
//          << " f" << serializer(node->data) <<" ";
//   }
// }
// template <class recType, class Metric>
// template <class Serializer>
// inline
// void Tree<recType,Metric>::serialize_aux(Node_ptr node, std::ostringstream &
// ostr, Serializer & serializer) {
//   std::stack<Node_ptr> nodeStack;
//   Node_ptr curNode = node;
//   nodeStack.push(curNode);
//   while(!nodeStack.empty()) {
//     curNode = nodeStack.top();
//     nodeStack.pop();
//     serialize_node(curNode, ostr, serializer);
//     for(auto & c : curNode->children) {
//       nodeStack.push(c);
//     }
//     ostr << ")";
//   }
// }
    template <class recType, class Metric>
    template <class Archive>
    inline void Tree<recType, Metric>::serialize_aux(Node_ptr node,
                                                     Archive &archive) {
        SerializedNode<recType, Metric> sn(node);
        if (node->children.size() > 0) {
            // sn.save(archive, 0);
            archive << SERIALIZATION_NVP2("node", sn);
            for (auto &c : node->children) {
                serialize_aux(c, archive);
            }
            SerializedNode<recType, Metric> snn(nullptr);
            archive << SERIALIZATION_NVP2("node", snn);
        } else {
            archive << SERIALIZATION_NVP2("node", sn);
        }
    }

    template <class recType, class Metric>
    template <class Archive>
    inline void Tree<recType, Metric>::serialize(Archive &archive) {
        std::shared_lock<std::shared_timed_mutex> lk(global_mut);
        (void)lk;
        //  std::ostringstream ostr;
        serialize_aux(root, archive);
        //  return ostr.str();
    }
    template <class recType, class Metric>
    template <class Archive>
    inline auto Tree<recType, Metric>::deserialize_node(Archive &istr)
        -> SerializedNode<recType, Metric> {
        SerializedNode<recType, Metric> n;
        istr &n;
        return n;
    }

    template <class recType, class Metric>
    template <class Archive, class Stream>
    inline void Tree<recType, Metric>::deserialize(Archive &input, Stream &stream) {
        SerializedNode<recType, Metric> node;
        std::unique_lock<std::shared_timed_mutex> lk(global_mut);
        (void)lk;

        try {
            input >> SERIALIZATION_NVP2("node", node);
            std::stack<Node_ptr> parentstack;
            parentstack.push(node.node);
            while (!stream.eof()) {
                // auto node = deserialize_node(input);
                SerializedNode<recType, Metric> node;
                // node.load(input, 0);
                input >> SERIALIZATION_NVP2("node", node);
                // input & snode;
                if (!node.is_null) {
                    parentstack.top()->children.push_back(node.node);
                    node.node->parent = parentstack.top();
                    if (node.has_children) {
                        parentstack.push(node.node);
                    }
                } else {
                    parentstack.pop();
                }
            }
        } catch (...) { /* hack to catch end of stream */
        }
        root = node.node;
    }
    template <class recType, class Metric>
    inline bool Tree<recType, Metric>::same_tree(const Node_ptr lhs,
                                                 const Node_ptr rhs) const {
        std::shared_lock<std::shared_timed_mutex> lk(global_mut);
        (void)lk;

        if (lhs == rhs) {
            return true;
        }
        if (lhs->ID != rhs->ID || lhs->level != rhs->level ||
            lhs->parent_dist != rhs->parent_dist || lhs->data != rhs->data) {
            return false;
        }

        if (lhs->children.size() != rhs->children.size())
            return false;
        for (std::size_t i = 0; i < lhs->children.size(); i++) {
            if (!same_tree(lhs->children[i], rhs->children[i]))
                return false;
        }
        return true;
    }

    template <typename recType, class Metric>
    inline Node<recType, Metric> *Tree<recType, Metric>::insert_(Node_ptr p,
                                                                 Node_ptr x) {
        auto children = sortChildrenByDistance(p, x);
        auto child_idx = std::get<0>(children);
        for (auto qi : child_idx) {
            auto &q = p->children[qi];
            if (q->dist(x) <= q->covdist()) {
                auto q1 = insert_(q, x);
                p->children[qi] = q1;
                q1->parent = p;
                q1->parent_dist = p->dist(q1);
                // for(std::size_t i =0; i <  p->children.size(); i++) {
                //   if(p->children[i] == q) {
                //     p->children[i] = q1;
                //     break;
                //   }
                //}
                return p;
            }
        }
        p->children.push_back(x);
        x->parent = p;
        x->parent_dist = p->dist(x);
        x->level = p->level - 1;
        return p;
        //  return rebalance(p,x);
    }

    template <typename recType, class Metric>
    inline auto Tree<recType, Metric>::rebalance(Node_ptr p, Node_ptr x)
        -> Node_ptr {
        x->level = p->level - 1;
        auto p1 = p;
        for (std::size_t i = 0; i < p->children.size(); i++) {
            rset_t res = rebalance_(p, p->children[i], x);
            Node_ptr q1 = std::get<0>(res);
            if (q1 != nullptr) {
                p1->children[i] = q1;
            }
            auto moveset = std::get<1>(res);
            for (auto r : moveset) {
                x = insert(x, r);
            }
        }
        p1->setChild(x);
        return p1;
    }
    template <typename recType, class Metric>
    inline auto Tree<recType, Metric>::rebalance_(Node_ptr p, Node_ptr q,
                                                  Node_ptr x) -> rset_t {
        if (p->dist(q) > q->dist(x)) {
            std::vector<Node_ptr> moveset;
            std::vector<Node_ptr> stayset;
            auto descendants = q->descendants();
            for (auto &r : descendants) {
                if (r->dist(p) > r->dist(x)) {
                    moveset.push_back(r);
                } else {
                    stayset.push_back(r);
                }
            }
            return std::make_tuple(nullptr, moveset, stayset);
        } else {
            std::vector<Node_ptr> moveset1;
            std::vector<Node_ptr> stayset1;
            auto q1 = q;
            for (auto it = q->children.begin(); it != q->children.end();) {
                auto r = *it;
                auto res = rebalance_(p, r, x);
                auto r1 = std::get<0>(res);
                auto moveset = std::get<1>(res);
                auto stayset = std::get<2>(res);
                moveset1.insert(moveset1.end(), moveset.begin(), moveset.end());
                stayset1.insert(stayset1.end(), stayset.begin(), stayset.end());
                if (r1 == nullptr) {
                    it = q->children.erase(it);
                } else {
                    *it = r1;
                    it++;
                }
            }
            for (auto it = stayset1.begin(); it != stayset1.end();) {
                auto r = *it;
                if (r->dist(q1) <= q1->covdist()) {
                    q1 = insert(q1, r);
                    it = stayset1.erase(it);
                } else {
                    it++;
                }
            }
            return std::make_tuple(q1, moveset1, stayset1);
        }
    }


    template <typename recType, typename Metric>
    inline double Tree<recType, Metric>::find_neighbour_radius(
        const std::vector<recType> &points) {
        double radius = std::numeric_limits<double>::min();
        auto &p1 = points[0];
//        Metric m;
        for (std::size_t i = 1; i < points.size(); i++) {
            double distance = metric(p1, points[i]);
            if (distance > radius)
                radius = distance;
        }
        return radius;
    }
    template <typename recType, typename Metric>
    inline double Tree<recType, Metric>::find_neighbour_radius(
        const std::vector<std::size_t> &IDS,
        const std::vector<recType> &points) {
        double radius = std::numeric_limits<double>::min();
        auto &p1 = points[IDS[0]];
//        Metric m;
        for (std::size_t i = 1; i < IDS.size(); i++) {
            double distance = metric(p1, points[IDS[i]]);
            if (distance > radius)
                radius = distance;
        }
        return radius;
    }

    template <typename recType, typename Metric>
    inline double Tree<recType, Metric>::find_neighbour_radius(
        const std::vector<std::size_t> &IDS) {
        double radius = std::numeric_limits<double>::min();
        auto &p1 = (*this)[IDS[0]];
        for (std::size_t i = 1; i < IDS.size(); i++) {
            double distance = metric(p1, (*this)[IDS[i]]);
            if (distance > radius)
                radius = distance;
        }
        return radius;
    }

//     template<typename T>
//     inline std::string print_vec(const std::vector<T> &v) {
//         std::ostringstream os;
//         os <<"[";
//         for(auto i : v) {

//             os << i << ",";
//         }
//         os << "]";
//         return os.str();
//     }
//     inline std::string print_vec_vec(const
//     std::vector<std::vector<std::size_t>> & v) {
//         std::ostringstream os;
//         os << "{ ";
//         for(auto & i : v) {
//             os << print_vec(i) << ", ";
//         }
//         os << " }";
//         return os.str();
//     }

    template <typename recType, typename Metric>
    inline bool Tree<recType, Metric>::update_idx(std::size_t &cur_idx,
                    const std::vector<std::size_t> &distribution_sizes,
                    std::vector<std::vector<std::size_t>> &result) {
        if (result[cur_idx].size() == distribution_sizes[cur_idx])
            cur_idx++;
        if (cur_idx == result.size())
            return true;
        return false;
    }
    template <typename recType, typename Metric>
    inline bool Tree<recType, Metric>::grab_sub_tree(
        Node_ptr proot, const recType &center,
        std::unordered_set<std::size_t> &parsed_points,
        const std::vector<std::size_t> &distribution_sizes, std::size_t &cur_idx,
        std::vector<std::vector<std::size_t>> &result) {
        auto childs = this->sortChildrenByDistance(proot, center);
        auto idx = std::get<0>(childs);
        auto dists = std::get<1>(childs);
        bool is_root_added = false;
        if (dists.empty() || dists[0] > proot->dist(center)) {
            if (parsed_points.find(proot->ID) == parsed_points.end()) {
                result[cur_idx].push_back(proot->ID);
                parsed_points.insert(proot->ID);
                is_root_added = true;
                if (update_idx(cur_idx, distribution_sizes, result))
                    return true;
            }
        }
        std::size_t index = 0;
        for (auto i : idx) {
            if (!is_root_added && dists[index] > proot->dist(center)) {
                if (parsed_points.find(proot->ID) == parsed_points.end()) {
                    result[cur_idx].push_back(proot->ID);
                    parsed_points.insert(proot->ID);
                    is_root_added = true;
                    if (update_idx(cur_idx, distribution_sizes, result))
                        return true;
                }
            }

            if (parsed_points.find(proot->children[i]->ID) == parsed_points.end())
                if (grab_sub_tree(proot->children[i], center, parsed_points,
                                  distribution_sizes, cur_idx, result))
                    return true;
            index++;
        }
        if (!is_root_added) {
            if (parsed_points.find(proot->ID) == parsed_points.end()) {
                result[cur_idx].push_back(proot->ID);
                parsed_points.insert(proot->ID);
                if (update_idx(cur_idx, distribution_sizes, result))
                    return true;
            }
        }
        return false;
    }

    template <typename recType, typename Metric>
    inline bool Tree<recType, Metric>::grab_tree(
        Node_ptr start_point, const recType &center,
        std::unordered_set<std::size_t> &parsed_points,
        const std::vector<std::size_t> &distribution_sizes, std::size_t &cur_idx,
        std::vector<std::vector<std::size_t>> &result) {
        if (grab_sub_tree(start_point, center, parsed_points, distribution_sizes,
                          cur_idx, result))
            return true;
        parsed_points.insert(start_point->ID);
        Node_ptr proot = start_point->parent;
        while (proot != nullptr) {
            if (grab_sub_tree(proot, center, parsed_points, distribution_sizes, cur_idx,
                              result))
                return true;
            proot = proot->parent;
        }
        return false;
    }

    inline void is_distribution_ok(const std::vector<double> & distribution) {
        if(distribution.empty())
            return;
        double d0 = distribution[0];
        for(std::size_t i = 1; i < distribution.size(); i++) {
            if(distribution[i] < d0)
                throw unsorted_distribution_exception{};
            if(distribution[i] < 0.0 || distribution[i] > 1.0)
                throw bad_distribution_exception{};
            d0 = distribution[i];
        }
        return;
    }
    template <typename recType, typename Metric>
    inline std::vector<std::vector<std::size_t>>
    Tree<recType, Metric>::clustering(const std::vector<double> &distribution,
                                      const std::vector<std::size_t> &IDS,
                                      const std::vector<recType> &points) {
        is_distribution_ok(distribution);
        double radius = find_neighbour_radius(IDS, points);
        return clustering_impl(distribution, points[IDS[0]], radius);
    }

    template <typename recType, typename Metric>
    inline std::vector<std::vector<std::size_t>>
    Tree<recType, Metric>::clustering(const std::vector<double> &distribution,
                                      const std::vector<std::size_t> &IDS) {
        is_distribution_ok(distribution);
        double radius = find_neighbour_radius(IDS);
        Node_ptr center = this->get(IDS[0]);
        return clustering_impl(distribution, center, radius);
    }

    template <typename recType, typename Metric>
    inline std::vector<std::vector<std::size_t>>
    Tree<recType, Metric>::clustering(const std::vector<double> &distribution,
                                      const std::vector<recType> &points) {
        is_distribution_ok(distribution);
        double radius = find_neighbour_radius(points);
        return clustering_impl(distribution, points[0], radius);
    }

    template <typename recType, typename Metric>
    inline std::vector<std::vector<std::size_t>>
    Tree<recType, Metric>::clustering_impl(const std::vector<double> &distribution,
                                           const recType &center, double radius) {
        std::vector<std::size_t> distribution_sizes;
        distribution_sizes.reserve(distribution.size());
        // std::vector<std::size_t> distribution_index(distribution.size());
        // std::iota(distribution_index.begin(), distribution_index.end(), 0);
        auto tree_size = size();
        for (auto d : distribution) {
            distribution_sizes.push_back(static_cast<double>(tree_size) * d);
        }
        // std::sort(distribution_index.begin(), distribution_index.end(),
        //           [&distribution_sizes](auto lhs, auto rhs) {
        //               return distribution_sizes[lhs] < distribution_sizes[rhs];
        //           });

        std::size_t ls = distribution_sizes[0];
        for (std::size_t i = 1; i < distribution_sizes.size(); i++) {
            auto ls1 = distribution_sizes[i];
            distribution_sizes[i] -= ls;
            ls = ls1;
        }

        auto proot = nn(center);
        int level = proot->level;
        double level_radius = std::pow(base, level);

        // find level covering all points
        while (level_radius < radius) {
            proot = proot->parent;
            level_radius = std::pow(base, proot->level);
        }
        std::size_t cur_distrib_idx = 0;
        std::vector<std::vector<std::size_t>> result(distribution.size());
        while (cur_distrib_idx < distribution_sizes.size() &&
               distribution_sizes[cur_distrib_idx] == 0) {
            cur_distrib_idx++;
        }
        if (cur_distrib_idx == distribution_sizes.size())
            return result;

        std::unordered_set<std::size_t> parsed_points;
        grab_tree(proot, center, parsed_points, distribution_sizes, cur_distrib_idx,
                  result);

        return result;
    }

    template <typename T> std::string convert_to_string(const T &t) {
        return std::to_string(t);
    }

#define DECLARE_CONVERT(type)                         \
    template <> inline                                 \
    std::string convert_to_string<std::vector<type>>( \
        const std::vector<type> &v) {                 \
        std::ostringstream ostr;                      \
        ostr << "[ ";                                 \
        for (std::size_t i = 0; i < v.size(); i++) {  \
            ostr << std::to_string(v[i]);             \
            if (i != v.size() - 1)                    \
                ostr << ",";                          \
        }                                             \
        ostr << " ]";                                 \
        return ostr.str();                            \
    }

    DECLARE_CONVERT(int)
    DECLARE_CONVERT(unsigned int)
    DECLARE_CONVERT(int64_t)
    DECLARE_CONVERT(uint64_t)
    DECLARE_CONVERT(double)
    DECLARE_CONVERT(float)
    DECLARE_CONVERT(char)

    template<typename recType, typename Metric>
    inline std::string Tree<recType, Metric>::to_json()  {
        return to_json([](const auto &r) { return convert_to_string(r);});
    }

    template <typename recType, typename Metric>
    inline std::string Tree<recType, Metric>::to_json(std::function<std::string(const recType&)> printer) {
        struct node_t {
            std::size_t ID;
            recType value;
        };
        struct edge_t {
            std::size_t source;
            std::size_t target;
            Distance distance;
        };
        std::vector<node_t> nodes;
        std::vector<edge_t> edges;
        traverse([&nodes, &edges](auto p) {
                     nodes.emplace_back(node_t{p->ID, p->data});
                     if (p->parent != nullptr) {
                         edges.emplace_back(edge_t{p->parent->ID, p->ID, p->parent_dist});
                     }
                 });
        std::ostringstream ostr;
        ostr << "{" << std::endl;
        ostr << "\"nodes\": [" << std::endl;
        for (std::size_t i = 0; i < nodes.size(); i++) {
            auto &n = nodes[i];
            ostr << "{ \"id\":" << n.ID << ", \"values\":" << printer(n.value)
                 << "}";
            if (i != nodes.size() - 1)
                ostr << ",";
            ostr << std::endl;
        }
        ostr << "]," << std::endl;
        ostr << "\"edges\": [" << std::endl;
        for (std::size_t i = 0; i < edges.size(); i++) {
            auto &n = edges[i];
            ostr << "{ \"source\":" << n.source << ", \"target\":" << n.target
                 << ", \"distance\":" << std::fixed << n.distance << "}";
            if (i != edges.size() - 1)
                ostr << ",";
            ostr << std::endl;
        }
        ostr << "]}" << std::endl;
        return ostr.str();
    }
} // namespace metric_search
