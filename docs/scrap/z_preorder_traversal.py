
from collections import deque

class Node:
    def __init__(self, node=None):
        self.parent = node
        self.children = []
        self.value = None






def linear_tree(levels):
    root = Node()
    root.value = (0,'>')
    
    stack = [root]
    
    while len(stack):
        node = stack.pop()
        
        level,name = node.value
        
        if level < levels:
            for i in range(8):
                child_node = Node(node)
                child_name = name + '.%s' % i
                child_node.value = (level+1, child_name)
                node.children += [child_node]
                stack.append(child_node)
    return root

def z_preorder_traverse(node0, visitor):

    siblings0 = tuple([node0])
    stack = deque([siblings0])
    
    while len(stack):
        print ('len(stack):', len(stack))
        nodes = stack.popleft()
        
        for node in nodes:
            if len(node.children) > 0:
                child_siblings = tuple(node.children)
                stack.append(child_siblings)
            visitor(node)


def z_preorder_traverse1(start_node, visitor):

    stack = deque([(0,start_node)])
    
    while len(stack):
        print ('len(stack):', len(stack))
        level,node0 = stack.pop()
        
        nodes = [node0]
        
        while len(stack):
            next_level, next_node = stack.pop()
            if next_level != level:
                stack.append( (next_level, next_node) )
            nodes += [next_node]
        
        for node in nodes:
            for child_node in node.children:
                stack.append( (level+1, child_node) )
            visitor(node)

def preorder_traverse(node0, visitor):

    parent_stack = []
    node = node0
    while len(parent_stack) > 0 or node is not None:
        #print ('parent_stack:', [pnode.value for pnode in parent_stack])
        if node is not None:
            visitor(node)
            
            if len(node.children) == 0:
                node = None
            else:
                next_node = node.children[0]
                for child in node.children[1:]:
                    parent_stack.append(child)
                node = next_node
        else:
            node = parent_stack.pop()


def z_preorder_traverse2(node0, visitor):
    visitor(node0)
    
    parent_stack = []
    node = node0
    while len(parent_stack) > 0 or node is not None:
        #print ('parent_stack:', [pnode.value for pnode in parent_stack])
        if node is not None:
            for child in node.children:
                visitor(child)
            
            if len(node.children) == 0:
                node = None
            else:
                next_node = node.children[0]
                for child in node.children[1:]:
                    parent_stack.append(child)
                node = next_node
        else:
            node = parent_stack.pop()
def print_node(node):
    print (node.value)


root = linear_tree(3)
z_preorder_traverse2(root, print_node)








