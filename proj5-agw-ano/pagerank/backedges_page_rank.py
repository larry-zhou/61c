from simple_page_rank import SimplePageRank

"""
This class implements the pagerank algorithm with
backwards edges as described in the second part of 
the project.
"""
class BackedgesPageRank(SimplePageRank):

    """
    The implementation of __init__ and compute_pagerank should 
    still be the same as SimplePageRank.
    You are free to override them if you so desire, but the signatures
    must remain the same.
    """

    """
    This time you will be responsible for implementing the initialization
    as well. 
    Think about what additional information your data structure needs 
    compared to the old case to compute weight transfers from pressing
    the 'back' button.
    """
    @staticmethod
    def initialize_nodes(input_rdd):

        def emit_edges(line):
            if len(line) == 0 or line[0] == "#":
                return []
            source, target = tuple(map(int, line.split()))
            edge = (source, frozenset([target]))
            self_source = (source, frozenset())
            self_target = (target, frozenset())
            return [edge, self_source, self_target]

        def reduce_edges(e1, e2):
            return e1 | e2

        def initialize_weights((source, targets)):
            return (source, (1.0, targets, 1.0))

        nodes = input_rdd\
                .flatMap(emit_edges)\
                .reduceByKey(reduce_edges)\
                .map(initialize_weights)
        return nodes

    """
    You will also implement update_weights and format_output from scratch.
    You may find the distribute and collect pattern from SimplePageRank
    to be suitable, but you are free to do whatever you want as long
    as it results in the correct output..
    """
    @staticmethod
    def update_weights(nodes, num_nodes):
        """
        Mapper phase.
        """
        def distribute_weights((node, (weight, targets, old))):
            
            temp = []
            if len(targets) == 0:
                me = []
                for n in range(num_nodes):
                    if n != node:
                        temp.append((n, (0.85 * weight/(num_nodes-1), [], 0)))
                targets = [-1]
                me.append((node, (0.05 * weight + .1 * old, targets, weight)))
                return me + temp
            temp.append((node, (0.05 * weight + .1 * old, targets, weight)))
            for targ in targets:
                temp.append((targ, (0.85 * weight/len(targets), [], 0)))
            return temp

        """
        Reducer phase.
        """
        def collect_weights((node, values)):
            edges = []
            weight = 0.0
            old = 0.0
            for val in values:
                weight += val[0]
                if val[1]:
                    old = val[2]
                    if type(val[1]) is frozenset:
                        edges = val[1]
            return (node, (weight, edges, old))
        return nodes\
                .flatMap(distribute_weights)\
                .groupByKey()\
                .map(collect_weights)
                     
    @staticmethod
    def format_output(nodes):
        return nodes\
                .map(lambda (node, (weight, targets, old)): (weight, node))\
                .sortByKey(ascending = False)\
                .map(lambda (weight, node): (node, weight))
