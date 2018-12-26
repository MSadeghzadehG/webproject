

package org.springframework.boot.autoconfigure.batch;

import java.util.ArrayList;
import java.util.List;

import org.springframework.batch.core.JobExecution;
import org.springframework.boot.ExitCodeGenerator;
import org.springframework.context.ApplicationListener;


public class JobExecutionExitCodeGenerator
		implements ApplicationListener<JobExecutionEvent>, ExitCodeGenerator {

	private final List<JobExecution> executions = new ArrayList<>();

	@Override
	public void onApplicationEvent(JobExecutionEvent event) {
		this.executions.add(event.getJobExecution());
	}

	@Override
	public int getExitCode() {
		for (JobExecution execution : this.executions) {
			if (execution.getStatus().ordinal() > 0) {
				return execution.getStatus().ordinal();
			}
		}
		return 0;
	}

}
