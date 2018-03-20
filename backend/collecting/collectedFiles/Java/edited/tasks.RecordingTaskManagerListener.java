

package org.elasticsearch.action.admin.cluster.node.tasks;

import org.elasticsearch.common.collect.Tuple;
import org.elasticsearch.common.regex.Regex;
import org.elasticsearch.tasks.Task;
import org.elasticsearch.tasks.TaskInfo;
import org.elasticsearch.test.tasks.MockTaskManagerListener;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.stream.Collectors;


public class RecordingTaskManagerListener implements MockTaskManagerListener {

    private String[] actionMasks;
    private String localNodeId;

    private List<Tuple<Boolean, TaskInfo>> events  = new ArrayList<>();

    public RecordingTaskManagerListener(String localNodeId, String... actionMasks) {
        this.actionMasks = actionMasks;
        this.localNodeId = localNodeId;
    }

    @Override
    public synchronized void onTaskRegistered(Task task) {
        if (Regex.simpleMatch(actionMasks, task.getAction())) {
            events.add(new Tuple<>(true, task.taskInfo(localNodeId, true)));
        }
    }

    @Override
    public synchronized void onTaskUnregistered(Task task) {
        if (Regex.simpleMatch(actionMasks, task.getAction())) {
            events.add(new Tuple<>(false, task.taskInfo(localNodeId, true)));
        }
    }

    @Override
    public void waitForTaskCompletion(Task task) {
    }

    public synchronized List<Tuple<Boolean, TaskInfo>> getEvents() {
        return Collections.unmodifiableList(new ArrayList<>(events));
    }

    public synchronized List<TaskInfo> getRegistrationEvents() {
        List<TaskInfo> events = this.events.stream().filter(Tuple::v1).map(Tuple::v2).collect(Collectors.toList());
        return Collections.unmodifiableList(events);
    }

    public synchronized List<TaskInfo> getUnregistrationEvents() {
        List<TaskInfo> events = this.events.stream().filter(event -> event.v1() == false).map(Tuple::v2).collect(Collectors.toList());
        return Collections.unmodifiableList(events);
    }

    public synchronized void reset() {
        events.clear();
    }

}
